#include <GpService/GpService.hpp>
#include <GpService/ArgParser/GpServiceArgParser.hpp>
#include <GpCore2/GpTasks/Fibers/Boost/GpStackImplPoolBoost.hpp>
#include <GpCore2/GpTasks/Fibers/GpTaskFiberCtxFactory.hpp>
#include <GpCore2/GpTasks/Fibers/Boost/GpTaskFiberCtxFactoryBoost.hpp>
#include <GpCore2/GpTasks/Scheduler/GpTaskScheduler.hpp>
#include <GpCore2/GpTasks/Scheduler/V1/GpTaskSchedulerV1Factory.hpp>
#include <GpCore2/GpTasks/ITC/GpItcFutureUtils.hpp>
#include <GpCore2/GpUtils/Files/GpFileUtils.hpp>
#include <GpCore2/GpUtils/Other/GpDefer.hpp>
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

GpService::UP               GpService::sServiceUP;
GpMutex<>                   GpService::sServiceMutex;
volatile std::sig_atomic_t  GpService::sSignalReceived  = 0;
GpConditionVarFlag          GpService::sStopFlagCV;
std::atomic<int>            GpService::sResultCode      = EXIT_SUCCESS;

GpService::GpService (std::string aName):
iName{std::move(aName)}
{
}

GpService::~GpService (void) noexcept
{
}

int GpService::SStartAndWaitForStop
(
    const std::string               aName,
    const size_t                    aArgc,
    const char* const               aArgv[],
    GpLogConsumersFactory::UP       aLogConsumersFactoryUP,
    const GpLogLevel::EnumT         aDefaultLogLevel,
    GpServiceCfgBaseDescFactory::UP aCfgBaseDescFactoryUP,
    GpServiceArgBaseDescFactory::UP aArgBaseDescFactoryUP,
    GpServiceMainTaskFactory::UP    aServiceMainTaskFactoryUP,
    GpServiceDelegate::UP           aServiceDelegateUP
)
{
#if defined(GP_OS_WINDOWS)
    SetConsoleOutputCP(CP_UTF8);
#endif// #if defined(GP_OS_WINDOWS)

    GpLog::SSetLevel(aDefaultLogLevel);
    GpThread::SSetSysNameForCurrent("Main");

    int exitCode = EXIT_FAILURE;

    GpDefer onStop = [&aName]()
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
    };

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
            GpUniqueLock uniqueLock{GpService::sServiceMutex};

            VERIFY
            (
                GpService::sServiceUP == nullptr,
                "GpService::SStart already called"_sv
            );

            GpService::sServiceUP = std::make_unique<GpService>(aName);
            startCode = GpService::sServiceUP->Start
            (
                NumOps::SConvert<size_t>(aArgc),
                aArgv,
                std::move(aLogConsumersFactoryUP),
                std::move(aCfgBaseDescFactoryUP),
                std::move(aArgBaseDescFactoryUP),
                std::move(aServiceMainTaskFactoryUP)
            );
        }

        GpService& service = *GpService::sServiceUP;
        exitCode = startCode;

        if (startCode != EXIT_FAILURE)
        {
            //const auto beginSTS = GpDateTimeOps::SSteadyTS_ms();

            GpTask::DoneFutureT& mainTaskDoneFuture = service.iMainTaskDoneFutureSP.V();

            // --- Wait for stop signal or for main task done ---
            bool done = false;

            if (aServiceDelegateUP)
            {
                aServiceDelegateUP->OnStart();
            }

            while (!done)
            {
                // Check if main task done
                if (mainTaskDoneFuture.IsReady())
                {
                    done = true;

                    auto doneFutureRes = mainTaskDoneFuture.ExtractResult();
                    if (doneFutureRes.IsException())
                    {
                        LOG_EXCEPTION("[GpService::SStartAndWaitForStop]", doneFutureRes.ExtractException());
                        exitCode = EXIT_FAILURE;
                    }
                }

                // Check OS signal
                done |= GpService::sSignalReceived != 0;

                // Check timeout
                /*
                {
                    const auto nowSTS = GpDateTimeOps::SSteadyTS_ms();
                    if ((nowSTS - beginSTS) > 3.0_si_s)
                    {
                        GpOutUtils::S().StdOut("[GpService::SStartAndWaitForStop]: Check timeout"_sv);
                        done = true;
                    }
                }
                */

                if (!done) [[likely]]
                {
                    if (aServiceDelegateUP)
                    {
                        aServiceDelegateUP->OnStep();
                    }

                    // Wait for
                    done |= sStopFlagCV.WaitFor(100.0_si_ms);
                }
            }
        }

        // --- Stop ---
        {
            GpUniqueLock uniqueLock{GpService::sServiceMutex};

            if (aServiceDelegateUP)
            {
                aServiceDelegateUP->OnStop();
            }

            const int stopExitCode = service.Stop();
            GpService::sServiceUP.reset();

            if (stopExitCode == EXIT_FAILURE)
            {
                exitCode = stopExitCode;
            }
        }
    } catch (const GpException& ex)
    {
        if (aServiceDelegateUP)
        {
            aServiceDelegateUP->OnError(ex);
        }

        LOG_EXCEPTION("[GpService::SStartAndWaitForStop]", ex);
        exitCode = EXIT_FAILURE;
    } catch (const std::exception& ex)
    {
        if (aServiceDelegateUP)
        {
            aServiceDelegateUP->OnError(ex);
        }

        LOG_EXCEPTION("[GpService::SStartAndWaitForStop]", GpException{ex.what()});
        exitCode = EXIT_FAILURE;
    } catch (...)
    {
        if (aServiceDelegateUP)
        {
            aServiceDelegateUP->OnError(GpException{"Unknown exception"});
        }

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
    GpLogLevel::EnumT               aDefaultLogLevel,
    GpServiceMainTaskFactory::UP    aServiceMainTaskFactoryUP
)
{
    // Create log factory
    GpLogConsumersFactory::UP logConsumersFactoryUP = std::make_unique<GpLogConsumersFactory>();
    logConsumersFactoryUP->AddDefaultProcessorConsole();

    return SStartAndWaitForStop
    (
        std::move(aName),
        0,
        nullptr,
        std::move(logConsumersFactoryUP),
        aDefaultLogLevel,
        std::make_unique<GpServiceCfgBaseDescFactory>(),
        std::make_unique<GpServiceArgBaseDescFactory>(),
        std::move(aServiceMainTaskFactoryUP),
        nullptr
    );
}

GpService::ResFutureT   GpService::SStartInSeparateThread
(
    std::string                     aName,
    size_t                          aArgc,
    const char* const               aArgv[],
    GpLogConsumersFactory::UP       aLogConsumersFactoryUP,
    GpLogLevel::EnumT               aDefaultLogLevel,
    GpServiceCfgBaseDescFactory::UP aCfgBaseDescFactoryUP,
    GpServiceArgBaseDescFactory::UP aArgBaseDescFactoryUP,
    GpServiceMainTaskFactory::UP    aServiceMainTaskFactoryUP,
    GpServiceDelegate::UP           aServiceDelegateUP
)
{
    GpThread::SSetSysNameForCurrent("Main");

    std::promise<int> promise;
    auto future = promise.get_future();

    auto threadFn =
    [
        promise                 = std::move(promise),
        name                    = std::move(aName),
        argc                    = aArgc,
        argv                    = aArgv,
        logConsumersFactoryUP   = std::move(aLogConsumersFactoryUP),
        defaultLogLevel         = aDefaultLogLevel,
        cfgBaseDescFactoryUP    = std::move(aCfgBaseDescFactoryUP),
        argBaseDescFactoryUP    = std::move(aArgBaseDescFactoryUP),
        serviceMainTaskFactoryUP= std::move(aServiceMainTaskFactoryUP),
        serviceDelegateUP       = std::move(aServiceDelegateUP)
    ]() mutable
    {
        try
        {
            const int res = GpService::SStartAndWaitForStop
            (
                std::move(name),
                argc,
                argv,
                std::move(logConsumersFactoryUP),
                defaultLogLevel,
                std::move(cfgBaseDescFactoryUP),
                std::move(argBaseDescFactoryUP),
                std::move(serviceMainTaskFactoryUP),
                std::move(serviceDelegateUP)
            );

            promise.set_value(res);
        } catch (...)
        {
            promise.set_exception(std::current_exception());
        }
    };

    std::thread thread(std::move(threadFn));
    thread.detach();

    return future;
}

void    GpService::SRequestStop (void) noexcept
{
    sStopFlagCV.UpFlagAndNotifyAll();
}

GpServiceArgBaseDesc::CSP   GpService::SArgs (void)
{
    return GpService::sServiceUP->iArgsDescSP;
}

GpArgParserRes::CSP GpService::SArgsParseRes (void)
{
    return GpService::sServiceUP->iArgsParseRes;
}

GpServiceCfgBaseDesc::CSP   GpService::SCfg (void)
{
    return GpService::sServiceUP->iCfgDescSP;
}

void    GpService::SSetResultCode (const int aCode)
{
    GpService::sResultCode = aCode;
}

std::string_view    GpService::SApplicationName (void) noexcept
{
    return GpService::sServiceUP->Name();
}

void    GpService::SSystemSignalsHandler (int aSignalId) noexcept
{
    std::cout << "[GpService::SSystemSignalsHandler]: !!!!!!!!!!!!!!! aSignalId: " << aSignalId << std::endl;
    std::cout.flush();

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

#if defined(GP_OS_WINDOWS)
BOOL WINAPI GpService::SWinConsoleHandler (DWORD aSignalId) noexcept
{
    std::cout << "[GpService::SWinConsoleHandler]: !!!!!!!!!!!!!!!" << std::endl;
    std::cout.flush();

    GpStackTrace::SPrintStacktrace
    (
        fmt::format
        (
            "[GpService::SWinConsoleHandler]: signal id = {}. ",
            aSignalId
        )
    );

    std::exit(EXIT_FAILURE);
}
#endif// #if defined(GP_OS_WINDOWS)

int GpService::Start
(
    const size_t                    aArgc,
    const char* const               aArgv[],
    GpLogConsumersFactory::UP       aLogConsumersFactoryUP,
    GpServiceCfgBaseDescFactory::UP aCfgBaseDescFactoryUP,
    GpServiceArgBaseDescFactory::UP aArgBaseDescFactoryUP,
    GpServiceMainTaskFactory::UP    aServiceMainTaskFactoryUP
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

#if defined(GP_LOCK_TRACE)
        iLockTraceThread.Start();
#endif// #if defined(GP_LOCK_TRACE)

        ParseServiceArgs(aArgc, aArgv, std::move(aArgBaseDescFactoryUP));
        ReadConfig(std::move(aCfgBaseDescFactoryUP));
        SetSystemSignalsHandler();
        StartLog(std::move(aLogConsumersFactoryUP));
        isLogStarted = true;
        StartFibers();
        StartTaskScheduler();
        StartMainTask(std::move(aServiceMainTaskFactoryUP));
    } catch (const GpException& ex)
    {
        if (isLogStarted)
        {
            LOG_EXCEPTION("[GpService::Start]", ex);
        } else
        {
            GpOutUtils::S().Err(ex.what());
        }

        startCode = EXIT_FAILURE;
    } catch (const std::exception& ex)
    {
        if (isLogStarted)
        {
            LOG_EXCEPTION("[GpService::Start]", GpException{ex.what()});
        } else
        {
            GpOutUtils::S().Err(ex.what());
        }

        startCode = EXIT_FAILURE;
    } catch (...)
    {
        if (isLogStarted)
        {
            LOG_EXCEPTION("[GpService::Start]", GpException{"Unknown exception"});
        } else
        {
            GpOutUtils::S().Err("[GpService::Start]: Unknown exception catched");
        }

        startCode = EXIT_FAILURE;
    }

    return startCode;
}

int GpService::Stop (void) noexcept
{
    int stopCode = GpService::sResultCode;

    try
    {
#if defined(GP_USE_TIMERS)
        GpTimersManager::SDisableShots();
#endif

        StopTaskScheduler();
        StopFibers();

#if defined(GP_USE_TIMERS)
        GpTimersManager::SStop();
#endif

#if defined(GP_LOCK_TRACE)
        iLockTraceThread.RequestStop();
        iLockTraceThread.Join();
#endif// #if defined(GP_LOCK_TRACE)
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

        VERIFY(pid > 0, "Fork failed (stage 2)"_sv);
    } else if (pid > 0)//ForkResT::PARENT
    {
        return ForkResT::PARENT;
    }

    THROW("Fork failed (stage 1)"_sv);
#else
    THROW("fork() not supported on current platform..."_sv);
#endif// #if defined(GP_POSIX)

    return ForkResT::CHILD;
}*/

void    GpService::ParseServiceArgs
(
    const size_t                    aArgc,
    const char* const               aArgv[],
    GpServiceArgBaseDescFactory::UP aArgBaseDescFactory
)
{
    std::tie(iArgsDescSP, iArgsParseRes) = GpServiceArgParser::SParse
    (
        aArgc,
        aArgv,
        *aArgBaseDescFactory
    );
}

void    GpService::ReadConfig (GpServiceCfgBaseDescFactory::UP  aCfgBaseDescFactory)
{
    iCfgDescSP = aCfgBaseDescFactory->NewInstance();

    std::string_view cfgFileName = iArgsDescSP.V().CfgFile();

    if (std::size(cfgFileName) > 0)
    {
        GpByteArray fileData    = GpFileUtils::SReadAll(cfgFileName);
        std::string fileDataStr = std::string(GpSpanCharR(fileData).AsStringView());
        GpJsonSerializer::SFromStrInsitu(fileDataStr, iCfgDescSP.Vn(), {});
    } else
    {
        auto& executors_cnt = iCfgDescSP.Vn().task_manager.executors_cnt;
        if (executors_cnt == 0)
        {
            executors_cnt = std::max<u_int_32>(1, std::thread::hardware_concurrency() - 1);
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
    SetConsoleCtrlHandler(SWinConsoleHandler, TRUE);

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

void    GpService::StartLog (GpLogConsumersFactory::UP aConsumersFactory)
{
    const GpServiceArgBaseDesc& argDesc         = iArgsDescSP.V();
    const GpServiceCfgBaseDesc& serviceCfg      = iCfgDescSP.V();
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
            100.0_si_ms,
            std::move(consumers)
        );
    }

    const GpLogConfigDesc& logCfgDesc = logCfgDescCSP.V();

    GpLog::S().StartFromConfig
    (
        logCfgDesc,
        argDesc.LogLevel(),
        *aConsumersFactory
    );
}

void    GpService::StartFibers (void)
{
#if defined(GP_USE_MULTITHREADING_FIBERS)
    const auto& serviceCfg      = iCfgDescSP.V();
    const auto& taskManagerCfg  = serviceCfg.task_manager;

    //
    GpTaskFiberCtxFactory::SInit(std::make_unique<GpTaskFiberCtxFactoryBoost>());

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
    const auto& serviceCfg      = iCfgDescSP.V();
    const auto& taskManagerCfg  = serviceCfg.task_manager;

    GpTaskScheduler::SStart
    (
        GpTaskSchedulerV1Factory{},
        taskManagerCfg.executors_cnt,
        taskManagerCfg.tasks_max_cnt
    );
}

void    GpService::StopTaskScheduler (void)
{
    if (iMainTaskSP.IsNotNULL())
    {
        // Send stop to main task
        std::ignore = iMainTaskSP->RequestStop();

        // Wait for main task done (from main thread)
        {
            auto& doneFuture = iMainTaskDoneFutureSP.Vn();

            while (!doneFuture.IsReady())
            {
                GP_ASM_SPIN_PAUSE();
            }

            auto res = doneFuture.ExtractResult();
            if (res.IsException())
            {
                LOG_EXCEPTION("Main task", res.ExtractException());
            }
        }
    }

    GpTaskScheduler::SStop();
}

void    GpService::StartMainTask (GpServiceMainTaskFactory::UP aServiceMainTaskFactoryUP)
{
    // Create main task
    iMainTaskSP = aServiceMainTaskFactoryUP->NewInstance();

    // Get futures
    iMainTaskStartFutureSP  = iMainTaskSP->StartFuture();
    iMainTaskDoneFutureSP   = iMainTaskSP->DoneFuture();

    // Start task
    SPAWN_READY_TASK(iMainTaskSP);

    // Wait for mait task start (from main thread)
    {
        auto& startFuture = iMainTaskStartFutureSP.Vn();

        while (!startFuture.IsReady())
        {
            GP_ASM_SPIN_PAUSE();
        }

        auto res = startFuture.ExtractResult();
        if (res.IsException())
        {
            throw res.ExtractException();
        }
    }
}

std::string_view    GpService::Name (void) const noexcept
{
    return iName;
}

}// namespace GPlatform
