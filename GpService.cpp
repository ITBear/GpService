#include <GpService/GpService.hpp>
#include <GpService/ArgParser/GpServiceArgParser.hpp>
#include <GpCore2/GpTasks/Fibers/Boost/GpStackImplPoolBoost.hpp>
#include <GpCore2/GpTasks/Fibers/GpTaskFiberCtxFactory.hpp>
#include <GpCore2/GpTasks/Fibers/Boost/GpTaskFiberCtxFactoryBoost.hpp>
#include <GpCore2/GpTasks/Scheduler/GpTaskScheduler.hpp>
#include <GpCore2/GpTasks/Scheduler/V1/GpTaskSchedulerV1Factory.hpp>
#include <GpCore2/GpTasks/ITC/GpItcSharedFutureUtils.hpp>
#include <GpCore2/GpUtils/Files/GpFileUtils.hpp>
#include <GpCore2/GpUtils/Other/GpRAIIonDestruct.hpp>
#include <GpCore2/GpUtils/Random/GpSRandom.hpp>
#include <GpCore2/GpUtils/Threads/GpThread.hpp>
#include <GpCore2/GpUtils/Threads/Timers/GpTimersManager.hpp>
#include <GpCore2/GpUtils/Debugging/GpStackTrace.hpp>
#include <GpJson/GpJsonSerializer.hpp>
#include <GpLog/GpLogCore/GpLog.hpp>
#include <GpLog/GpLogCore/Consumers/Console/GpLogConsumerConsoleConfigDesc.hpp>

#include <iostream>
#include <signal.h>

#if defined(GP_POSIX)
#   include <unistd.h>
#   include <execinfo.h>
#endif

namespace GPlatform {

GpService::SP               GpService::sService;
std::mutex                  GpService::sServiceMutex;
GpConditionVarFlag          GpService::sServiceCondVar;
std::atomic_flag            GpService::sIsStopRequested;
volatile std::sig_atomic_t  GpService::sSignalReceived  = 0;

GpService::GpService (std::string aName):
iName{std::move(aName)}
{
}

GpService::~GpService (void) noexcept
{
}

int GpService::SStartAndWaitForStop
(
    std::string                     aName,
    const size_t                    aArgc,
    const char* const               aArgv[],
    GpLogConsumersFactory::SP       aLogConsumersFactory,
    const GpLogLevel::EnumT         aDefaultLogLevel,
    GpServiceCfgBaseDescFactory::SP aCfgBaseDescFactory,
    GpServiceArgBaseDescFactory::SP aArgBaseDescFactory,
    GpServiceMainTaskFactory::SP    aServiceMainTaskFactory
)
{
    GpLog::SSetLevel(aDefaultLogLevel);
    GpThread::SSetSysNameForCurrent("Main");

    int exitCode = EXIT_FAILURE;

    GpRAIIonDestruct onStop
    (
        [aName]()
        {
            const milliseconds_t appStopSteadyTS    = GpDateTimeOps::SSteadyTS_ms();
            const milliseconds_t appRunTime         = appStopSteadyTS - GpDateTimeOps::SSteadyTS_ms_AtAppStart();

            LOG_INFO
            (
                fmt::format
                (
                    ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> STOP application: \"{}\" <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" \
                    "\nTotal run time: {}",
                    aName,
                    GpDateTimeOps::SToDaysHoursMinSec(appRunTime)
                )
            );

            GpLog::S().Stop();
            GpLog::SClear();

            std::cout << "\n";

            std::cout.flush();
            std::clog.flush();
            std::cerr.flush();
        }
    );

    try
    {
        // Init log
        GpLog::SInit();

        LOG_INFO
        (
            fmt::format
            (
                ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> START application: \"{}\" <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<",
                aName
            )
        );

        // Write system info to log
        LOG_SYS_INFO("------------ System info ------------");

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
                // Check if stop requested
                done |= sIsStopRequested.test();

                // Check if main task done
                done |= mainTaskDoneFuture.IsReady();

                // Check OS signal
                done |= GpService::sSignalReceived != 0;

                // Check timeout
                //{
                //  const auto nowSTS = GpDateTimeOps::SSteadyTS_ms();
                //  if ((nowSTS - beginSTS) > 3.0_si_s)
                //  {
                //      GpStringUtils::SCout("[GpService::SStartAndWaitForStop]: Check timeout"_sv);
                //      done = true;
                //  }
                //}

                if (!done) [[likely]]
                {
                    // Wait for
                    GpService::sServiceCondVar.WaitForAndReset(0.1_si_s);
                }
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
    } catch (const GpException& ex)
    {
        LOG_EXCEPTION("[GpService::SStartAndWaitForStop]", ex);
        exitCode = EXIT_FAILURE;
    } catch (const std::exception& ex)
    {
        LOG_EXCEPTION("[GpService::SStartAndWaitForStop]", GpException{ex.what()});
        exitCode = EXIT_FAILURE;
    } catch (...)
    {
        LOG_EXCEPTION("GpService::SStartAndWaitForStop", GpException{"Unknown exception"});
        exitCode = EXIT_FAILURE;
    }

    LOG_INFO
    (
        fmt::format
        (
            "[GpService::SStartAndWaitForStop]: Done (exit code {})",
            exitCode
        )
    );

    return exitCode;
}

int GpService::SStartAndWaitForStop
(
    std::string                     aName,
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
        GpLogLevel::L_DEBUG,
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

GpArgParserRes::CSP GpService::SArgsParseRes (void)
{
    return GpService::sService.V().iArgsParseRes;
}

void    GpService::SInterruptWaitForStop (void) noexcept
{
    //GpStringUtils::SCout("[GpService::SInterruptWaitForStop]: begin");
    GpService::sServiceCondVar.NotifyAll();
    //GpStringUtils::SCout("[GpService::SInterruptWaitForStop]: end");
}

void    GpService::SSystemSignalsHandler (int aSignalId) noexcept
{
    switch (aSignalId)
    {
#if defined(GP_POSIX)
        case SIGHUP:
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
#endif
        case SIGTERM:// A termination request sent to the program.
        {
            GpService::sSignalReceived = aSignalId;
        } break;
        case SIGINT:// Receipt of an interactive attention signal.
        {
            GpService::sSignalReceived = aSignalId;
        } break;
        case SIGABRT:   [[fallthrough]];    // Abnormal termination of the program, such as a call to abort.
        case SIGFPE:    [[fallthrough]];    // An erroneous arithmetic operation, such as a divide by zero or an operation resulting in overflow
        case SIGILL:    [[fallthrough]];    // Detection of an illegal instruction.
        case SIGSEGV:   [[fallthrough]];
        default:
        {
            GpStackTrace::SPrintStacktrace
            (
                fmt::format
                (
                    "[GpService::SSystemSignalsHandler]: signal id = {}. ",
                    aSignalId
                )
            );

            std::exit(EXIT_FAILURE);
        }
    }
}

int GpService::Start
(
    const size_t                    aArgc,
    const char* const               aArgv[],
    GpLogConsumersFactory::SP       aLogConsumersFactory,
    GpServiceCfgBaseDescFactory::SP aCfgBaseDescFactory,
    GpServiceArgBaseDescFactory::SP aArgBaseDescFactory,
    GpServiceMainTaskFactory::SP    aServiceMainTaskFactory
) noexcept
{
    int     startCode       = EXIT_SUCCESS;
    bool    isLogStarted    = false;

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
        isLogStarted = true;
        StartFibers();
        StartTaskScheduler();
        StartMainTask(aServiceMainTaskFactory);
    } catch (const GpException& ex)
    {
        if (isLogStarted)
        {
            LOG_EXCEPTION("[GpService::Start]", ex);
        } else
        {
            GpStringUtils::SCerr(ex.what());
        }

        startCode = EXIT_FAILURE;
    } catch (const std::exception& ex)
    {
        if (isLogStarted)
        {
            LOG_EXCEPTION("[GpService::Start]", GpException{ex.what()});
        } else
        {
            GpStringUtils::SCerr(ex.what());
        }

        startCode = EXIT_FAILURE;
    } catch (...)
    {
        if (isLogStarted)
        {
            LOG_EXCEPTION("[GpService::Start]", GpException{"Unknown exception"});
        } else
        {
            GpStringUtils::SCerr("[GpService::Start]: Unknown exception catched");
        }

        startCode = EXIT_FAILURE;
    }

    return startCode;
}

int GpService::Stop (void) noexcept
{
    int stopCode = EXIT_SUCCESS;

    try
    {
        //GpStringUtils::SCout("[GpService::Stop]: stop...");

#if defined(GP_USE_TIMERS)
        //GpStringUtils::SCout("[GpService::Stop]: GpTimersManager::SDisableShots()...");
        GpTimersManager::SDisableShots();
#endif

        //GpStringUtils::SCout("[GpService::Stop]: StopTaskScheduler()...");
        StopTaskScheduler();

        //GpStringUtils::SCout("[GpService::Stop]: StopFibers()...");
        StopFibers();

#if defined(GP_USE_TIMERS)
        //GpStringUtils::SCout("[GpService::Stop]: GpTimersManager::SStop()...");
        GpTimersManager::SStop();
#endif

        //GpStringUtils::SCout("[GpService::Stop]: done...");
    } catch (const GpException& ex)
    {
        LOG_EXCEPTION("[GpService::Stop]", ex);
        stopCode = EXIT_FAILURE;
    } catch (const std::exception& ex)
    {
        LOG_EXCEPTION("[GpService::Stop]", GpException{ex.what()});
        stopCode = EXIT_FAILURE;
    } catch (...)
    {
        LOG_EXCEPTION("[GpService::Stop]", GpException{"Unknown exception"});
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

    THROW_GP("Fork failed (stage 1)"_sv);
#else
    THROW_GP("fork() not supported on current platform..."_sv);
#endif// #if defined(GP_POSIX)

    return ForkResT::CHILD;
}*/

void    GpService::ParseServiceArgs
(
    const size_t                    aArgc,
    const char* const               aArgv[],
    GpServiceArgBaseDescFactory::SP aArgBaseDescFactory
)
{
    std::tie(iArgsDesc, iArgsParseRes) = GpServiceArgParser::SParse
    (
        aArgc,
        aArgv,
        aArgBaseDescFactory.V()
    );
}

void    GpService::ReadConfig (GpServiceCfgBaseDescFactory::SP  aCfgBaseDescFactory)
{
    iCfgDesc = aCfgBaseDescFactory->NewInstance();

    std::string_view cfgFileName = iArgsDesc.V().CfgFile();

    if (std::size(cfgFileName) > 0)
    {
        GpBytesArray    fileData    = GpFileUtils::SReadAll(cfgFileName);
        std::string     fileDataStr = std::string(GpSpanCharR(fileData).AsStringView());
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

    sa.sa_handler   = SSystemSignalsHandler;
    sa.sa_flags     = 0;

    sigemptyset(&sa.sa_mask);

    sigaction(SIGHUP,   &sa, nullptr);
    sigaction(SIGTERM,  &sa, nullptr);
    sigaction(SIGINT,   &sa, nullptr);
    sigaction(SIGKILL,  &sa, nullptr);
    sigaction(SIGSEGV,  &sa, nullptr);
    sigaction(SIGPIPE,  &sa, nullptr);

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
            GpStackTrace::SPrintStacktrace("[std::terminate]: ");
            std::abort();
        }
    );
}

void    GpService::StartLog (GpLogConsumersFactory::SP aConsumersFactory)
{
    const GpServiceArgBaseDesc& argDesc         = iArgsDesc.V();

    const GpServiceCfgBaseDesc& serviceCfg      = iCfgDesc.V();
    GpLogConfigDesc::CSP        logCfgDescCSP   = serviceCfg.log;

    if (logCfgDescCSP.IsNULL())
    {
        // Create default config for log (only console output)
        GpLogConsumerConfigDesc::C::MapStr::SP consumers =
        {
            {"console", MakeSP<GpLogConsumerConsoleConfigDesc>()}
        };

        logCfgDescCSP = MakeCSP<GpLogConfigDesc>
        (
            std::min(argDesc.LogLevel(), GpLog::SLevel()),
            0.1_si_s,
            std::move(consumers)
        );
    }

    const GpLogConfigDesc& logCfgDesc = logCfgDescCSP.V();

    GpLog::S().StartFromConfig
    (
        logCfgDesc,
        argDesc.LogLevel(),
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
        GpTaskSchedulerV1Factory{},
        taskManagerCfg.executors_cnt,
        taskManagerCfg.tasks_max_cnt,
        []()
        {
            GpService::SRequestStop();
        }
    );
}

void    GpService::StopTaskScheduler (void)
{
    if (iMainTaskSP.IsNotNULL())
    {
        //GpStringUtils::SCout("[GpService::Stop]: Send stop to main task...");

        // Send stop to main task
        iMainTaskSP->RequestTaskStop();

        // Wait for main task done
        GpItcSharedFutureUtils::SWaitForInf
        (
            iMainTaskDoneFuture.Vn(),
            [&](typename GpTaskFiber::DoneFutureT::value_type&)// OnSuccessFnT
            {
                // NOP
            },
            [](const GpException&)// OnExceptionFnT
            {
                // NOP
                // LOG_EXCEPTION("Main task", aEx);
            },
            100.0_si_ms
        );
    }

    GpTaskScheduler::SStopAndClear();
}

void    GpService::StartMainTask (GpServiceMainTaskFactory::SP aServiceMainTaskFactory)
{
    // Create main task
    iMainTaskSP = aServiceMainTaskFactory->NewInstance
    (
        iArgsDesc.V(),
        iCfgDesc.V()
    );

    // Get futures
    iMainTaskStartFuture    = iMainTaskSP->GetStartFuture();
    iMainTaskDoneFuture     = iMainTaskSP->GetDoneFuture();

    // Start task
    GpTaskScheduler::S().NewToReady(iMainTaskSP);

    // Wait for start
    GpItcSharedFutureUtils::SWaitForInf
    (
        iMainTaskStartFuture.V(),
        [&](typename GpTaskFiber::StartFutureT::value_type&)//OnSuccessFnT
        {
            // NOP
        },
        [](const GpException& aEx)//OnExceptionFnT
        {
            throw aEx;
        },
        100.0_si_ms
    );
}

}// namespace GPlatform
