#include "GpService.hpp"
#include "ArgParser/GpArgParser.hpp"

#include <iostream>
#include <signal.h>

#if defined(GP_POSIX)
#   include <unistd.h>
#endif

namespace GPlatform {

std::mutex      GpService::sServiceMutex;
GpService::SP   GpService::sService;
GpConditionVar  GpService::sServiceCondVar;

GpService::GpService (std::string_view aName):
iName(aName)
{
}

GpService::~GpService (void) noexcept
{
}

int GpService::SRun (const size_t                       aArgc,
                     char**                             aArgv,
                     const GpServiceFactory::SP         aServiceFactory,
                     const GpTaskFactory::C::Vec::SP&   aTaskFactories) noexcept
{
    try
    {
        THROW_GPE_COND
        (
            GpService::sService.IsNULL(),
            "GpService::SStart already called"_sv
        );

        GpService::sService = aServiceFactory->NewInstance();
        GpService::sService->Start(NumOps::SConvert<size_t>(aArgc), aArgv, aTaskFactories);
        SWaitForInterrupt();
        return SStop();
    } catch (const std::exception& e)
    {
        GpExceptionsSink::SSink(e);
        SStop();
    } catch (...)
    {
        GpExceptionsSink::SSinkUnknown();
        SStop();
    }

    return EXIT_FAILURE;
}

void    GpService::SWaitForInterrupt (void) noexcept
{
    GpService::sServiceCondVar.WaitForWakeup();
    //std::this_thread::sleep_for(std::chrono::seconds(3));
}

void    GpService::SInterrupt (void) noexcept
{
    GpService::sServiceCondVar.WakeupAll();
}

int GpService::SStop (void) noexcept
{
    try
    {
        std::scoped_lock lock(GpService::sServiceMutex);

        if (GpService::sService.IsNotNULL())
        {
            {
                GpService& service = GpService::sService.Vn();
                service.Stop();
            }

            GpService::sService.Clear();
        }

        return EXIT_SUCCESS;
    } catch (const std::exception& e)
    {
        GpExceptionsSink::SSink(e);
    } catch (...)
    {
        GpExceptionsSink::SSinkUnknown();
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
            SInterrupt();
        } break;
        case SIGPIPE:
        {
            std::runtime_error e("SIGPIPE received");
            GpExceptionsSink::SSink(e);
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

void    GpService::Start (const size_t                      aArgc,
                          char**                            aArgv,
                          const GpTaskFactory::C::Vec::SP&  aTaskFactories)
{
    GpSRandom::S().SetSeedFromRD();

    ParseCmdLineArgs(aArgc, aArgv);
    ReadConfig();
    SetSystemSignalsHandler();
    StartTaskScheduler();
    StartMainTask();
    StartTasks(aTaskFactories);
}

void    GpService::Stop (void)
{
    StopTaskScheduler();
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

        THROW_GPE_COND(pid > 0, "Fork failed (stage 2)"_sv);
    } else if (pid > 0)//ForkResT::PARENT
    {
        return ForkResT::PARENT;
    }

    THROW_GPE("Fork failed (stage 1)"_sv);
#else
    THROW_GPE("fork() not supported on current platform..."_sv);
#endif//#if defined(GP_POSIX)

    return ForkResT::CHILD;
}*/

void    GpService::ParseCmdLineArgs (const size_t aArgc, char** aArgv)
{
    iCmdLineArgsDesc = CreateCmdLineArgsDesc();
    GpArgParser::SParse(aArgc, aArgv, iCmdLineArgsDesc.V(), "Service arguments"_sv);
}

void    GpService::ReadConfig (void)
{
    iServiceCfgDesc = CreateServiceCfgDesc();

    std::string_view cfgFileName = iCmdLineArgsDesc->CfgFile();

    if (cfgFileName.length() > 0)
    {
        GpBytesArray    fileData    = GpFileUtils::SReadAll(cfgFileName);
        std::string     fileDataStr = std::string(GpRawPtrCharR(fileData).AsStringView());
        GpJsonMapper::SFromJsonInsitu(fileDataStr, iServiceCfgDesc.V());
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

void    GpService::StartTaskScheduler (void)
{
    const auto& serviceCfg      = iServiceCfgDesc.VCn();
    const auto& taskManagerCfg  = serviceCfg.TaskManager();

    GpTaskFiberManager::S().Init(taskManagerCfg.MaxFibersCnt(),
                                 taskManagerCfg.FiberStackSize());

    iTaskScheduler = MakeSP<GpTaskScheduler>();
    iTaskScheduler.Vn().Start(iTaskScheduler,
                              taskManagerCfg.ExecutorsCnt());
}

void    GpService::StopTaskScheduler (void) noexcept
{
    if (iTaskScheduler.IsNULL())
    {
        return;
    }

    iTaskScheduler.Vn().RequestStop();
    iTaskScheduler.Vn().Join();
    iTaskScheduler.Clear();
}

void    GpService::StartMainTask (void)
{
    GpServiceMainTask::SP mainTask = CreateMainTask(iCmdLineArgsDesc.VC(), iServiceCfgDesc);
    iTaskScheduler->AddTaskToReady(mainTask);

    //TODO: add dependencies to main task
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
}

void    GpService::StartTasks (const GpTaskFactory::C::Vec::SP& aTaskFactories)
{
    for (const GpTaskFactory::SP& tf: aTaskFactories)
    {
        iTaskScheduler->AddTaskToReady(tf->NewInstance());
    }
}

}//namespace GPlatform

/*
<?xml version="1.0" encoding="UTF-8"?>
<body xmlns="buyerInfoResponse">





    <balance>
        <balance balanceTypeId="0">231</balance>
        <activeBalance balanceTypeId="0">231</activeBalance>
        <inactiveBalance balanceTypeId="0">0</inactiveBalance>
        <balance balanceTypeId="15270">0</balance>
        <activeBalance balanceTypeId="15270">0</activeBalance>
        <inactiveBalance balanceTypeId="15270">0</inactiveBalance>
        <oddMoneyBalance>414</oddMoneyBalance>
        <oddMoneyFlags>0</oddMoneyFlags>
    </balance>


</body>

<?xml version="1.0" encoding="UTF-8"?>
<body *xmlns="buyerInfoResponse">





   <balance/>
</body>
*/
