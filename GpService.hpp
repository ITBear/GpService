#pragma once

#include "GpServiceMainTask.hpp"
#include "GpServiceFactory.hpp"

namespace GPlatform {

class GPSERVICE_API GpService
{
public:
    CLASS_REMOVE_CTRS(GpService)
    CLASS_DECLARE_DEFAULTS(GpService)

    enum class ForkResT
    {
        CHILD,
        PARENT
    };

protected:
                                        GpService               (std::string_view aName);

public:
    virtual                             ~GpService              (void) noexcept;

    [[nodiscard]] static int            SRun                    (const size_t                       aArgc,
                                                                 char**                             aArgv,
                                                                 const GpServiceFactory::SP         aServiceFactory,
                                                                 const GpTaskFactory::C::Vec::SP&   aTaskFactories) noexcept;

protected:
    static void                         SWaitForInterrupt       (void) noexcept;
    static void                         SInterrupt              (void) noexcept;
    static int                          SStop                   (void) noexcept;
    static void                         SSystemSignalsHandler   (int aSignalId) noexcept;

protected:
    virtual GpArgBaseDesc::SP           CreateCmdLineArgsDesc   (void) = 0;
    virtual GpServiceCfgBaseDesc::SP    CreateServiceCfgDesc    (void) = 0;
    virtual GpServiceMainTask::SP       CreateMainTask          (const GpArgBaseDesc&       aCmdLineArgsDesc,
                                                                 GpServiceCfgBaseDesc::CSP  aServiceCfgDesc) = 0;

private:
    void                                Start                   (const size_t                       aArgc,
                                                                 char**                             aArgv,
                                                                 const GpTaskFactory::C::Vec::SP&   aTaskFactories);
    void                                Stop                    (void);
    //ForkResT                          Fork                    (void);
    void                                ParseCmdLineArgs        (const size_t aArgc, char** aArgv);
    void                                ReadConfig              (void);
    void                                SetSystemSignalsHandler (void); 
    void                                StartTaskScheduler      (void);
    void                                StopTaskScheduler       (void) noexcept;

    void                                StartMainTask           (void);
    void                                StartTasks              (const GpTaskFactory::C::Vec::SP& aTaskFactories);

private:
    const std::string                   iName;
    GpArgBaseDesc::SP                   iCmdLineArgsDesc;
    GpServiceCfgBaseDesc::SP            iServiceCfgDesc;

    GpTaskScheduler::SP                 iTaskScheduler;

    static std::mutex                   sServiceMutex;
    static GpService::SP                sService;
    static GpConditionVar               sServiceCondVar;
};

}//GPlatform
