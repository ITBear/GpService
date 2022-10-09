#include "GpService.hpp"
#include "ArgParser/GpArgParser.hpp"
#include "../GpJson/GpJson.hpp"

#include <iostream>

#include <signal.h>

#if defined(GP_POSIX)
#   include <unistd.h>
#endif

namespace GPlatform {

std::mutex      GpService::sServiceMutex;
GpService::SP   GpService::sService;
GpConditionVar  GpService::sServiceCondVar;

GpService::~GpService (void) noexcept
{
}

int GpService::SRun
(
    const size_t                    aArgc,
    char**                          aArgv,
    const GpServiceFactory::SP      aServiceFactory,
    std::string                     aMainTaskName,
    const GpServiceStartFlags       aFlags,
    const GpLogConsumersFactory&    aLogConsumersFactory
)
{
    int exitCode = EXIT_FAILURE;

    GpRAIIonDestruct stopLog
    (
        []()
        {
            LOG_INFO
            (
                "================================================================================\n" \
                "=--------------------------------- Stop application ---------------------------=\n" \
                "================================================================================\n\n"_sv
            );

            GpLog::S().Stop();

            std::cout.flush();
        }
    );

    LOG_INFO
    (
        "================================================================================\n" \
        "=--------------------------------- Start application --------------------------=\n" \
        "================================================================================"_sv
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
            std::move(aMainTaskName),
            aFlags,
            aLogConsumersFactory
        );

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

    LOG_INFO("[GpService::SRun]: Done (exit code "_sv + exitCode + ")"_sv);

    const int stopCode = SStop();

    if (stopCode == EXIT_FAILURE)
    {
        exitCode = EXIT_FAILURE;
    }

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
    std::ignore = GpService::sServiceCondVar.WaitForWakeup();
    //std::this_thread::sleep_for(std::chrono::seconds(6));
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
            std::cerr << "[GpService::SSystemSignalsHandler]: SIGPIPE received" << std::endl;
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
    std::string                     aMainTaskName,
    const GpServiceStartFlags       aFlags,
    const GpLogConsumersFactory&    aLogConsumersFactory
)
{
    GpSRandom::S().SetSeedFromRD();

    ParseCmdLineArgs(aArgc, aArgv);
    ReadConfig(aFlags);
    SetSystemSignalsHandler();
    StartLog(aLogConsumersFactory);
    StartTaskScheduler(aFlags);
    StartMainTask(std::move(aMainTaskName));
}

void    GpService::Stop (void)
{
    StopTaskScheduler();

    //Check main task fitire
    if (iMainTaskFuture.IsNotNULL())
    {
        GpItcFuture& f = iMainTaskFuture.Vn();
        if (f.IsReady())
        {
            GpItcResult& fres = f.Result().V();

            if (fres.IsException())
            {
                throw std::any_cast<GpException>(fres.Value());
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

    THROW_GP("Fork failed (stage 1)"_sv);
#else
    THROW_GP("fork() not supported on current platform..."_sv);
#endif//#if defined(GP_POSIX)

    return ForkResT::CHILD;
}*/

void    GpService::ParseCmdLineArgs
(
    const size_t    aArgc,
    char**          aArgv
)
{
    iCmdLineArgsDesc = CreateCmdLineArgsDesc();
    GpArgParser::SParse(aArgc, aArgv, iCmdLineArgsDesc.V(), "Service arguments"_sv);
}

void    GpService::ReadConfig (const GpServiceStartFlags aFlags)
{
    iServiceCfgDesc = CreateServiceCfgDesc();

    std::string_view cfgFileName = iCmdLineArgsDesc.V().Cfg();

    if (cfgFileName.length() > 0)
    {
        GpBytesArray    fileData    = GpFileUtils::SReadAll(cfgFileName);
        std::string     fileDataStr = std::string(GpSpanPtrCharR(fileData).AsStringView());
        GpJsonSerializer::SFromStrInsitu(fileDataStr, iServiceCfgDesc.V(), {});
    } else
    {
        THROW_COND_GP
        (
            aFlags.Test(GpServiceStartFlag::CONFIG_REQUIRED) == false,
            "Set config file name: --cfg file_name"_sv
        );
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
}

void    GpService::StartLog (const GpLogConsumersFactory& aConsumersFactory)
{
    const auto&             serviceCfg  = iServiceCfgDesc.V();
    const GpLogConfigDesc&  logCfgDesc  = serviceCfg.log.V();

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
        taskManagerCfg.MaxFibersCnt(),
        taskManagerCfg.FiberStackSize()
    );

    GpTaskScheduler::S().Start(taskManagerCfg.ExecutorsCnt());
}

void    GpService::StopTaskScheduler (void) noexcept
{
    GpTaskScheduler& taskScheduler = GpTaskScheduler::S();

    taskScheduler.RequestStop();
    taskScheduler.Join();

    GpTaskFiberManagerEx::SClear();
}

void    GpService::StartMainTask (std::string aName)
{
    GpItcPromise    startPromise;
    GpItcFuture::SP startFuture = startPromise.Future();

    GpServiceMainTask::SP mainTask = CreateMainTask
    (
        std::move(aName),
        std::move(startPromise),
        iCmdLineArgsDesc.V(),
        iServiceCfgDesc.V()
    );

    iMainTaskFuture = mainTask.Vn().Future();

    GpTaskScheduler::S().NewToReady(std::move(mainTask));

    startFuture.V().Wait();

    GpItcResult::SP futureRes = startFuture.Vn().Result();

    GpItcResult::SExtract<size_t, size_t>
    (
        futureRes,
        [](size_t&& aRes) -> size_t
        {
            return std::move(aRes);
        },
        [](std::string_view aError) -> size_t
        {
            THROW_GP(aError);
        }
    );
}

}//namespace GPlatform
