#include "GpService.hpp"
#include "../GpCore2/GpTasks/Fibers/Boost/GpStackImplPoolBoost.hpp"
#include "../GpCore2/GpTasks/Fibers/GpTaskFiberCtxFactory.hpp"
#include "../GpCore2/GpTasks/Fibers/Boost/GpTaskFiberCtxFactoryBoost.hpp"
#include "../GpCore2/GpTasks/Scheduler/GpTaskScheduler.hpp"
#include "../GpCore2/GpTasks/Scheduler/V1/GpTaskSchedulerV1Factory.hpp"
#include "../GpCore2/GpUtils/Files/GpFileUtils.hpp"
#include "../GpCore2/GpUtils/Other/GpRAIIonDestruct.hpp"
#include "../GpCore2/GpUtils/Random/GpSRandom.hpp"
#include "../GpCore2/GpUtils/Threads/GpThread.hpp"
#include "../GpCore2/GpUtils/Threads/Timers/GpTimersManager.hpp"
#include "../GpJson/GpJsonSerializer.hpp"
#include "../GpLog/GpLogCore/GpLog.hpp"
#include "ArgParser/GpServiceArgParser.hpp"

#include <iostream>
#include <signal.h>

#if defined(GP_POSIX)
#   include <unistd.h>
#   include <execinfo.h>
#endif

namespace GPlatform {

GP_IMPLEMENT_LIB(GpServiceLib)

GpService::SP       GpService::sService;
std::mutex          GpService::sServiceMutex;
GpConditionVarFlag  GpService::sServiceCondVar;
std::atomic_flag    GpService::sIsStopRequested = false;

#if defined(GP_POSIX)
volatile std::sig_atomic_t  GpService::sSignalReceived = 0;
#endif// #if defined(GP_POSIX)

GpService::~GpService (void) noexcept
{
}

int GpService::SStartAndWaitForStop
(
    std::u8string                   aName,
    const size_t                    aArgc,
    char**                          aArgv,
    GpLogConsumersFactory::SP       aLogConsumersFactory,
    GpServiceCfgBaseDescFactory::SP aCfgBaseDescFactory,
    GpServiceArgBaseDescFactory::SP aArgBaseDescFactory,
    GpServiceMainTaskFactory::SP    aServiceMainTaskFactory
)
{
    GpThread::SSetSysNameForCurrent(u8"Main");

    int exitCode = EXIT_FAILURE;

    GpRAIIonDestruct onStop
    (
        []()
        {
            const milliseconds_t appStopSteadyTS    = GpDateTimeOps::SSteadyTS_ms();
            const milliseconds_t appRunTime         = appStopSteadyTS - GpDateTimeOps::SSteadyTS_ms_AtAppStart();

            LOG_INFO
            (
                  u8"================================================================================\n" \
                  u8"=--------------------------------- Stop application ---------------------------=\n"_sv
                + u8"= Total run time: "_sv + StrOps::SFromDouble(appRunTime.Value() / 1000.0) + u8" seconds\n" \
                  u8"================================================================================\n\n"_sv
            );

            GpLog::S().Stop();
            GpLog::SClear();

            std::cout.flush();
            std::clog.flush();
            std::cerr.flush();
        }
    );

    // Init log
    GpLog::SInit();

    LOG_INFO
    (
        u8"================================================================================\n" \
        u8"=--------------------------------- Start application --------------------------=\n" \
        u8"================================================================================"_sv
    );

    // Write system info to log
    LOG_SYS_INFO(u8"System info");

    try
    {
        int startCode = EXIT_SUCCESS;

        // --- Start ---
        {
            std::scoped_lock lock(GpService::sServiceMutex);

            THROW_COND_GP
            (
                GpService::sService.IsNULL(),
                "GpService::SStart already called"_sv
            );

            GpService::sService = MakeSP<GpService>(aName);

            startCode = GpService::sService.Vn().Start
            (
                NumOps::SConvert<size_t>(aArgc),
                aArgv,
                aLogConsumersFactory,
                aCfgBaseDescFactory,
                aArgBaseDescFactory,
                aServiceMainTaskFactory
            );
        }

        exitCode = startCode;

        if (startCode != EXIT_FAILURE)
        {
            //const auto beginSTS = GpDateTimeOps::SSteadyTS_ms();
            GpTask::DoneFutureT& mainTaskDoneFuture = GpService::sService->iMainTaskDoneFuture.V();

            // --- Wait for stop signal or for main task done ---
            bool done = false;

            while (!done)
            {
                // Wait for
                GpService::sServiceCondVar.WaitForAndReset(0.2_si_s);

                // Check if stop requested
                if (sIsStopRequested.test()) [[unlikely]]
                {
                    GpStringUtils::SCout(u8"[GpService::SStartAndWaitForStop]: sIsStopRequested.test() == true");
                    done = true;
                }

                // Check if main task done
                if (mainTaskDoneFuture.IsReady()) [[unlikely]]
                {
                    GpStringUtils::SCout(u8"[GpService::SStartAndWaitForStop]: mainTaskDoneFuture.IsReady() == true");
                    done = true;
                }

                // Check OS signal
                if (GpService::sSignalReceived != 0)
                {
                    GpStringUtils::SCout(u8"[GpService::SStartAndWaitForStop]: OS signal received: "_sv + int(GpService::sSignalReceived));
                    done = true;
                }

                // Check timeout
                //{
                //  const auto nowSTS = GpDateTimeOps::SSteadyTS_ms();
                //  if ((nowSTS - beginSTS) > 7.0_si_s)
                //  {
                //      GpStringUtils::SCout(u8"[GpService::SStartAndWaitForStop]: Check timeout"_sv);
                //      done = true;
                //  }
                //}
            }
        }

        // --- Stop ---
        {
            std::scoped_lock lock(GpService::sServiceMutex);

            if (GpService::sService.IsNotNULL())
            {
                const int stopExitCode = GpService::sService.V().Stop();
                GpService::sService.Clear();

                if (stopExitCode == EXIT_FAILURE)
                {
                    exitCode = stopExitCode;
                }
            }
        }
    } catch (const GpException& e)
    {
        LOG_EXCEPTION(e);
        exitCode = EXIT_FAILURE;
    } catch (const std::exception& e)
    {
        LOG_EXCEPTION(e);
        exitCode = EXIT_FAILURE;
    } catch (...)
    {
        LOG_EXCEPTION();
        exitCode = EXIT_FAILURE;
    }

    LOG_INFO(u8"[GpService::SRun]: Done (exit code "_sv + exitCode + u8")"_sv);

    GpStringUtils::SCout(u8"[GpService::SStartAndWaitForStop]: return exitCode = "_sv + exitCode);

    return exitCode;
}

int GpService::SStartAndWaitForStop
(
    std::u8string                   aName,
    GpServiceMainTaskFactory::SP    aServiceMainTaskFactory
)
{
    // Create log factory
    GpLogConsumersFactory::SP logConsumersFactorySP = MakeSP<GpLogConsumersFactory>();
    logConsumersFactorySP->AddDefaultProcessorConsole();

    return SStartAndWaitForStop
    (
        std::move(aName),
        0,
        nullptr,
        logConsumersFactorySP,
        MakeSP<GpServiceCfgBaseDescFactory>(),
        MakeSP<GpServiceArgBaseDescFactory>(),
        std::move(aServiceMainTaskFactory)
    );
}

void    GpService::SRequestStop (void) noexcept
{
    SInterruptWaitForStop();
    sIsStopRequested.test_and_set();
}

GpServiceArgBaseDesc::C::Opt::CRef  GpService::SArgs (void)
{
    const auto& r = GpService::sService.V().iArgsDesc;

    if (r.IsNULL())
    {
        return std::nullopt;
    }

    return r.V();
}

void    GpService::SInterruptWaitForStop (void) noexcept
{
    GpStringUtils::SCout("[GpService::SInterruptWaitForStop]: begin");
    GpService::sServiceCondVar.NotifyAll();
    GpStringUtils::SCout("[GpService::SInterruptWaitForStop]: end");
}

void    GpService::SPrintStacktrace (void)
{
#if defined(GP_POSIX)
    void*       ptrs[100];
    const int   size = backtrace(ptrs, 100);

    fprintf(stderr, "Error: signal SIGSEGV:\n");
    backtrace_symbols_fd(ptrs, size, STDERR_FILENO);
#else
#   error Unsupported platform
#endif
}

void    GpService::SSystemSignalsHandler (int aSignalId) noexcept
{
#if defined(GP_POSIX)
    switch (aSignalId)
    {
        case SIGHUP:
        {
            GpService::sSignalReceived = aSignalId;
        } break;
        case SIGTERM:// A termination request sent to the program.
        {
            GpService::sSignalReceived = aSignalId;
        } break;
        case SIGINT:// Receipt of an interactive attention signal.
        {
            GpService::sSignalReceived = aSignalId;
        } break;
        case SIGKILL:
        {
            GpService::sSignalReceived = aSignalId;
        } break;
        case SIGPIPE:
        {
            //NOP
        } break;
        case SIGABRT:   [[fallthrough]];    //Abnormal termination of the program, such as a call to abort.
        case SIGFPE:    [[fallthrough]];    //An erroneous arithmetic operation, such as a divide by zero or an operation resulting in overflow
        case SIGILL:    [[fallthrough]];    //Detection of an illegal instruction.
        case SIGSEGV:   [[fallthrough]];
        default:
        {
            SPrintStacktrace();
            std::exit(EXIT_FAILURE);
        }
    }
#elif defined(GP_OS_WINDOWS)
    switch (aSignalId)
    {
        case SIGTERM:   [[fallthrough]];    //A termination request sent to the program.
        case SIGINT:                        //Receipt of an interactive attention signal.
        {
            SInterrupt();
        } break;
        case SIGABRT:   [[fallthrough]];    //Abnormal termination of the program, such as a call to abort.
        case SIGFPE:    [[fallthrough]];    //An erroneous arithmetic operation, such as a divide by zero or an operation resulting in overflow
        case SIGILL:    [[fallthrough]];    //Detection of an illegal instruction.
        case SIGSEGV:   [[fallthrough]];    //An invalid access to storage.
        default:
        {
            std::exit(EXIT_FAILURE);
        }
    }
#else
#   error Unsupported platform
#endif
}

int GpService::Start
(
    const size_t                    aArgc,
    char**                          aArgv,
    GpLogConsumersFactory::SP       aLogConsumersFactory,
    GpServiceCfgBaseDescFactory::SP aCfgBaseDescFactory,
    GpServiceArgBaseDescFactory::SP aArgBaseDescFactory,
    GpServiceMainTaskFactory::SP    aServiceMainTaskFactory
) noexcept
{
    int startCode = EXIT_SUCCESS;

    try
    {
#if defined(GP_USE_TIMERS)
        GpTimersManager::SStart();
#endif

        GpSRandom::S().SetSeedFromRD();

        ParseServiceArgs(aArgc, aArgv, aArgBaseDescFactory);
        ReadConfig(aCfgBaseDescFactory);
        SetSystemSignalsHandler();
        StartLog(aLogConsumersFactory);     
        StartFibers();
        StartTaskScheduler();
        StartMainTask(aServiceMainTaskFactory);
    } catch (const GpException& e)
    {
        LOG_EXCEPTION(e);
        startCode = EXIT_FAILURE;
    } catch (const std::exception& e)
    {
        LOG_EXCEPTION(e);
        startCode = EXIT_FAILURE;
    } catch (...)
    {
        LOG_EXCEPTION();
        startCode = EXIT_FAILURE;
    }

    return startCode;
}

int GpService::Stop (void) noexcept
{
    int stopCode = EXIT_SUCCESS;

    try
    {
        GpStringUtils::SCout("[GpService::Stop]: stop...");

#if defined(GP_USE_TIMERS)
        GpStringUtils::SCout("[GpService::Stop]: GpTimersManager::SStop()...");
        GpTimersManager::SStop();
#endif

        GpStringUtils::SCout("[GpService::Stop]: StopTaskScheduler()...");
        StopTaskScheduler();

        GpStringUtils::SCout("[GpService::Stop]: CheckMainTaskDoneResult()...");
        CheckMainTaskDoneResult();

        GpStringUtils::SCout("[GpService::Stop]: StopFibers()...");
        StopFibers();

        GpStringUtils::SCout("[GpService::Stop]: done...");
    } catch (const GpException& e)
    {
        LOG_EXCEPTION(e);
        stopCode = EXIT_FAILURE;
    } catch (const std::exception& e)
    {
        LOG_EXCEPTION(e);
        stopCode = EXIT_FAILURE;
    } catch (...)
    {
        LOG_EXCEPTION();
        stopCode = EXIT_FAILURE;
    }

    return stopCode;
}

/*GpService::ForkResT   GpService::Fork (void)
{
#if defined(GP_POSIX)
    pid_t pid = fork();

    if (pid == 0)//ForkResT::CHILD
    {
        setsid();
        pid_t pid2 = fork();

        if (pid2 == 0)
        {
            return ForkResT::CHILD;
        }

        THROW_COND_GP(pid > 0, "Fork failed (stage 2)"_sv);
    } else if (pid > 0)//ForkResT::PARENT
    {
        return ForkResT::PARENT;
    }

    THROW_GP(u8"Fork failed (stage 1)"_sv);
#else
    THROW_GP(u8"fork() not supported on current platform..."_sv);
#endif//#if defined(GP_POSIX)

    return ForkResT::CHILD;
}*/

void    GpService::ParseServiceArgs
(
    const size_t                    aArgc,
    char**                          aArgv,
    GpServiceArgBaseDescFactory::SP aArgBaseDescFactory
)
{
    iArgsDesc = GpServiceArgParser::SParse
    (
        aArgc,
        aArgv,
        aArgBaseDescFactory.V(),
        u8"Service command line arguments"_sv
    );
}

void    GpService::ReadConfig (GpServiceCfgBaseDescFactory::SP  aCfgBaseDescFactory)
{
    iCfgDesc = aCfgBaseDescFactory->NewInstance();

    std::u8string_view cfgFileName = iArgsDesc.V().Cfg();

    if (cfgFileName.length() > 0)
    {
        GpBytesArray    fileData    = GpFileUtils::SReadAll(cfgFileName);
        std::u8string   fileDataStr = std::u8string(GpSpanPtrCharU8R(fileData).AsStringViewU8());
        GpJsonSerializer::SFromStrInsitu(fileDataStr, iCfgDesc.V(), {});
    } else
    {
        auto& executors_cnt = iCfgDesc.V().task_manager.executors_cnt;
        if (executors_cnt == 0)
        {
            executors_cnt = std::max<size_t>(1, std::thread::hardware_concurrency() - 1);
        }
    }
}

void    GpService::SetSystemSignalsHandler (void)
{
#if defined(GP_POSIX)
    struct sigaction sa;
    //std::memset(&sa, 0, sizeof(sa));
    sa.sa_handler   = SSystemSignalsHandler;
    sa.sa_flags     = 0;//SA_RESTART;

    sigemptyset(&sa.sa_mask);

    sigaction(SIGHUP,   &sa, nullptr);
    sigaction(SIGTERM,  &sa, nullptr);
    sigaction(SIGINT,   &sa, nullptr);
    sigaction(SIGKILL,  &sa, nullptr);
    sigaction(SIGSEGV,  &sa, nullptr);
    sigaction(SIGPIPE,  &sa, nullptr);

    //signal(SIGPIPE, SIG_IGN);
#elif defined(GP_OS_WINDOWS)
    signal(SIGTERM, SSystemSignalsHandler);
    signal(SIGINT,  SSystemSignalsHandler);
    signal(SIGABRT, SSystemSignalsHandler);
    signal(SIGFPE,  SSystemSignalsHandler);
    signal(SIGILL,  SSystemSignalsHandler);
    signal(SIGSEGV, SSystemSignalsHandler);
#else
#   error Unsupported platform
#endif

    std::set_terminate
    (
        []()
        {
            SPrintStacktrace();
            std::abort();
        }
    );
}

void    GpService::StartLog (GpLogConsumersFactory::SP aConsumersFactory)
{
    const GpServiceCfgBaseDesc& serviceCfg      = iCfgDesc.V();
    GpLogConfigDesc::CSP        logCfgDescCSP   = serviceCfg.log;

    if (logCfgDescCSP.IsNULL())
    {
        // Create default config for log (only console output)
        GpLogConsumerConfigDesc::C::MapStr::SP consumers =
        {
            {u8"console", MakeSP<GpLogConsumerConsoleConfigDesc>()}
        };

        logCfgDescCSP = MakeCSP<GpLogConfigDesc>
        (
            GpLogLevel::INFO,
            0.1_si_s,
            std::move(consumers)
        );
    }

    const GpLogConfigDesc& logCfgDesc = logCfgDescCSP.V();

    GpLog::S().StartFromConfig
    (
        logCfgDesc,
        aConsumersFactory.V()
    );
}

void    GpService::StartFibers (void)
{
#if defined(GP_USE_MULTITHREADING_FIBERS)
    const auto& serviceCfg      = iCfgDesc.V();
    const auto& taskManagerCfg  = serviceCfg.task_manager;

    //
    GpTaskFiberCtxFactory::SInit(MakeSP<GpTaskFiberCtxFactoryBoost>());

    //
    GpStackImplPoolBoost::S().Configure
    (
        taskManagerCfg.fibers_max_cnt,
        taskManagerCfg.fiber_stack_size
    );
#endif// #if defined(GP_USE_MULTITHREADING_FIBERS)
}

void    GpService::StopFibers (void)
{
#if defined(GP_USE_MULTITHREADING_FIBERS)
    //
    GpStackImplPoolBoost::S().Clear();

    //
    GpTaskFiberCtxFactory::SClear();
#endif// #if defined(GP_USE_MULTITHREADING_FIBERS)
}

void    GpService::StartTaskScheduler (void)
{
    const auto& serviceCfg      = iCfgDesc.V();
    const auto& taskManagerCfg  = serviceCfg.task_manager;

    GpTaskScheduler::SStart
    (
        GpTaskSchedulerV1Factory(),
        taskManagerCfg.executors_cnt,
        taskManagerCfg.tasks_max_cnt
    );
}

void    GpService::StopTaskScheduler (void)
{
    GpTaskScheduler::SStopAndClear();
}

void    GpService::StartMainTask (GpServiceMainTaskFactory::SP aServiceMainTaskFactory)
{
    // Create main task
    GpServiceMainTask::SP mainTask = aServiceMainTaskFactory->NewInstance
    (
        iArgsDesc.V(),
        iCfgDesc.V()
    );

    // Get futures
    iMainTaskStartFuture    = mainTask->GetStartFuture();
    iMainTaskDoneFuture     = mainTask->GetDoneFuture();

    // Start task
    GpTaskScheduler::S().NewToReady(std::move(mainTask));

    // Wait for started
    while (!iMainTaskStartFuture.Vn().WaitFor(100.0_si_ms))
    {
        // NOP
    }

    // Check start result
    GpTask::StartFutureT::SCheckIfReady
    (
        iMainTaskStartFuture.V(),
        [&](typename GpTaskFiber::StartFutureT::value_type&)//OnSuccessFnT
        {
            // NOP
        },
        [](const GpException& aEx)//OnExceptionFnT
        {
            throw aEx;
        }
    );
}

void    GpService::CheckMainTaskDoneResult (void)
{
    if (iMainTaskDoneFuture.IsNULL())
    {
        return;
    }

    // Wait for main task done
    while (!iMainTaskDoneFuture.Vn().WaitFor(100.0_si_ms))
    {
        // NOP
    }

    // Check done result
    GpTask::StartFutureT::SCheckIfReady
    (
        iMainTaskDoneFuture.V(),
        [&](typename GpTaskFiber::StartFutureT::value_type&)//OnSuccessFnT
        {
            // NOP
        },
        [](const GpException& aEx)//OnExceptionFnT
        {
            throw aEx;
        }
    );
}

}//namespace GPlatform
