#include "GpService.hpp"
#include "ArgParser/GpArgParser.hpp"
#include "../GpJson/GpJsonSerializer.hpp"
#include "../GpLog/GpLogCore/Consumers/GpLogConsumersFactory.hpp"
#include "../GpLog/GpLogCore/GpLog.hpp"
#include "../GpCore2/GpUtils/Other/GpRAIIonDestruct.hpp"
#include "../GpCore2/GpUtils/Threads/Timers/GpTimersManager.hpp"
#include "../GpCore2/GpUtils/Random/GpSRandom.hpp"
#include "../GpCore2/GpUtils/Files/GpFileUtils.hpp"
#include "../GpCore2/GpTasks/Fibers/GpTaskFiberManagerEx.hpp"
#include "../GpCore2/GpTasks/GpTaskScheduler.hpp"

#include <iostream>

#include <signal.h>

#if defined(GP_POSIX)
#   include <unistd.h>
#   include <execinfo.h>
#endif

namespace GPlatform {

std::mutex          GpService::sServiceMutex;
GpService::SP       GpService::sService;
GpConditionVarFlag  GpService::sServiceCondVar;

GpService::~GpService (void) noexcept
{
}

int GpService::SRun
(
    const size_t                        aArgc,
    char**                              aArgv,
    const GpServiceFactory::SP          aServiceFactory,
    const GpServiceStartFlags           aFlags,
    const GpLogConsumersFactory&        aLogConsumersFactory,
    const GpTaskFactory::C::Vec::SP&    aTasks
)
{
    int exitCode = EXIT_FAILURE;


    GpRAIIonDestruct stopLog
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

            std::cout.flush();
        }
    );

    LOG_INFO
    (
        u8"================================================================================\n" \
        u8"=--------------------------------- Start application --------------------------=\n" \
        u8"================================================================================"_sv
    );

    try
    {
        THROW_COND_GP
        (
            GpService::sService.IsNULL(),
            "GpService::SStart already called"_sv
        );

#if defined(GP_USE_TIMERS)
        GpTimersManager::SStart();
#endif

        GpService::sService = aServiceFactory.V().NewInstance();

        GpService::sService.Vn().Start
        (
            NumOps::SConvert<size_t>(aArgc),
            aArgv,
            aFlags,
            aLogConsumersFactory
        );

        SStartTasks(aTasks);

        SWaitForStop(aFlags);
        exitCode = EXIT_SUCCESS;
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

    const int stopCode = SStop();

    if (stopCode == EXIT_FAILURE)
    {
        exitCode = EXIT_FAILURE;
    }

    LOG_INFO(u8"[GpService::SRun]: Done (exit code "_sv + exitCode + u8")"_sv);

    return exitCode;
}

void    GpService::SRequestStop (void) noexcept
{
    SInterruptWaitForStop();
}

GpArgBaseDesc::C::Opt::CRef GpService::SArgs (void)
{
    const auto& r = GpService::sService.V().iCmdLineArgsDesc;

    if (r.IsNULL())
    {
        return std::nullopt;
    }

    return r.V();
}

void    GpService::SWaitForStop (const GpServiceStartFlags /*aFlags*/) noexcept
{
    GpService::sServiceCondVar.Wait();
    //std::this_thread::sleep_for(std::chrono::seconds(4));
}

void    GpService::SInterruptWaitForStop (void) noexcept
{
    GpService::sServiceCondVar.WakeupAll();
}

int GpService::SStop (void) noexcept
{
    try
    {
        std::scoped_lock lock(GpService::sServiceMutex);

#if defined(GP_USE_TIMERS)
        GpTimersManager::SStop();
#endif

        if (GpService::sService.IsNotNULL())
        {
            GpService::sService.V().Stop();
            GpService::sService.Clear();
        }

        return EXIT_SUCCESS;
    } catch (const std::exception& e)
    {
        LOG_EXCEPTION(e);
    } catch (...)
    {
        LOG_EXCEPTION();
    }

    return EXIT_FAILURE;
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
        case SIGHUP:    [[fallthrough]];
        case SIGTERM:   [[fallthrough]];    //A termination request sent to the program.
        case SIGINT:    [[fallthrough]];    //Receipt of an interactive attention signal.
        case SIGKILL:
        {
            SInterruptWaitForStop();
        } break;
        case SIGPIPE:
        {
            GpStringUtils::SCerr("[GpService::SSystemSignalsHandler]: SIGPIPE received\n");
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

void    GpService::Start
(
    const size_t                    aArgc,
    char**                          aArgv,
    const GpServiceStartFlags       aFlags,
    const GpLogConsumersFactory&    aLogConsumersFactory
)
{
    GpSRandom::S().SetSeedFromRD();

    ParseCmdLineArgs(aArgc, aArgv);
    ReadConfig(aFlags);
    SetSystemSignalsHandler();
    StartLog(aLogConsumersFactory, aFlags);
    StartTaskScheduler(aFlags);
    StartMainTask();
}

void    GpService::Stop (void)
{
    StopTaskScheduler();

    //Check main task fitire
    if (iMainTaskFuture.IsNotNULL())
    {
        StartItcFutureT& f = iMainTaskFuture.Vn();
        if (f.IsReady())
        {
            StartItcFutureT::ItcResultOptT fresOpt = f.Result();

            if (fresOpt.has_value())
            {
                const StartItcFutureT::ItcResultT::SP& fresSP = fresOpt.value();

                if (fresSP.IsNotNULL())
                {
                    const StartItcFutureT::ItcResultT& fres = fresSP.Vn();
                    if (fres.IsException())
                    {
                        const auto[msg, sourceLocation] = fres.Exception();
                        throw GpException(msg, sourceLocation);
                    }
                }
            }
        }
    }
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

void    GpService::ParseCmdLineArgs
(
    const size_t    aArgc,
    char**          aArgv
)
{
    GpArgBaseDescFactory::SP factory = CreateCmdLineArgsDescFactory();
    iCmdLineArgsDesc = GpArgParser::SParse(aArgc, aArgv, factory.V(), u8"Service arguments"_sv);
}

void    GpService::ReadConfig (const GpServiceStartFlags aFlags)
{
    iServiceCfgDesc = CreateServiceCfgDesc();

    std::u8string_view cfgFileName = iCmdLineArgsDesc.V().Cfg();

    if (cfgFileName.length() > 0)
    {
        GpBytesArray    fileData    = GpFileUtils::SReadAll(cfgFileName);
        std::u8string   fileDataStr = std::u8string(GpSpanPtrCharR(fileData).AsStringViewU8());
        GpJsonSerializer::SFromStrInsitu(fileDataStr, iServiceCfgDesc.V(), {});
    } else
    {
        THROW_COND_GP
        (
            aFlags.Test(GpServiceStartFlag::CONFIG_REQUIRED) == false,
            "Set config file name: --cfg file_name"_sv
        );

        iServiceCfgDesc.V().task_manager.executors_cnt = std::max<size_t>(1, std::thread::hardware_concurrency() - 1);
    }
}

void    GpService::SetSystemSignalsHandler (void)
{
#if defined(GP_POSIX)
    struct sigaction sa;
    std::memset(&sa, 0, sizeof(sa));
    sa.sa_handler   = SSystemSignalsHandler;
    sa.sa_flags     = SA_RESTART;

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

void    GpService::StartLog
(
    const GpLogConsumersFactory&    aConsumersFactory,
    const GpServiceStartFlags       aFlags
)
{
    const GpServiceCfgBaseDesc& serviceCfg      = iServiceCfgDesc.V();
    GpLogConfigDesc::CSP        logCfgDescCSP   = serviceCfg.log;

    if (aFlags.Test(GpServiceStartFlag::CONFIG_REQUIRED))
    {
        THROW_COND_GP
        (
            logCfgDescCSP.IsNotNULL(),
            "Log settings is null. Set --cfg argument with config file path or add log settings to config file"_sv
        );
    } else
    {
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

    const GpLogConfigDesc&  logCfgDesc  = logCfgDescCSP.V();

    GpLog::S().StartFromConfig
    (
        logCfgDesc,
        aConsumersFactory
    );
}

void    GpService::StartTaskScheduler (const GpServiceStartFlags /*aFlags*/)
{
    const auto& serviceCfg      = iServiceCfgDesc.V();
    const auto& taskManagerCfg  = serviceCfg.task_manager;

    GpTaskFiberManagerEx::SInit
    (
        taskManagerCfg.max_fibers_cnt,
        taskManagerCfg.fiber_stack_size
    );

    GpTaskScheduler::S().Start(taskManagerCfg.executors_cnt);
}

void    GpService::StopTaskScheduler (void) noexcept
{
    GpTaskScheduler& taskScheduler = GpTaskScheduler::S();

    taskScheduler.RequestStop();
    taskScheduler.Join();

    GpTaskFiberManagerEx::SClear();
}

void    GpService::StartMainTask (void)
{
    StartItcPromiseT    startPromise;
    StartItcFutureT::SP startFuture = startPromise.Future(GpTask::SCurrentUID());

    GpServiceMainTask::SP mainTask = CreateMainTask
    (
        std::move(startPromise),
        iCmdLineArgsDesc.V(),
        iServiceCfgDesc.V()
    );

    iMainTaskFuture = mainTask.Vn().Future();

    GpTaskScheduler::S().NewToReady(std::move(mainTask));

    startFuture.V().Wait();

    StartItcFutureT::ItcResultOptT futureRes = startFuture.Vn().Result();
    StartItcResultT::SExtract
    (
        futureRes,
        [](const size_t&)
        {
            //NOP
        },
        [](const auto& aError)
        {
            THROW_GP(std::get<0>(aError), std::get<1>(aError));
        }
    );
}

void    GpService::SStartTasks (const GpTaskFactory::C::Vec::SP& aTasks)
{
    for (const auto& factory: aTasks)
    {
        GpTask::SP task = factory.V().NewInstance(u8"Start Subtask");
        GpTaskScheduler::S().NewToReady(std::move(task));
    }
}

}//namespace GPlatform
