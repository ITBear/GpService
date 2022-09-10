#pragma once

#include "GpServiceMainTask.hpp"
#include "GpServiceFactory.hpp"
#include "GpServiceStartFlag.hpp"
#include "GpServiceCfgBaseDesc.hpp"
#include "ArgParser/GpArgParser.hpp"

namespace GPlatform {

class GP_SERVICE_API GpService
{
public:
    CLASS_REMOVE_CTRS_DEFAULT_MOVE_COPY(GpService)
    CLASS_DECLARE_DEFAULTS(GpService)

    enum class ForkResT
    {
        CHILD,
        PARENT
    };

protected:
    inline                              GpService               (std::string aName) noexcept;

public:
    virtual                             ~GpService              (void) noexcept;

    [[nodiscard]] static int            SRun                    (const size_t                   aArgc,
                                                                 char**                         aArgv,
                                                                 const GpServiceFactory::SP     aServiceFactory,
                                                                 std::string                    aMainTaskName,
                                                                 const GpServiceStartFlags      aFlags,
                                                                 const GpLogConsumersFactory&   aLogConsumersFactory);
    static void                         SRequestStop            (void) noexcept;
    static GpArgBaseDesc::C::Opt::CRef  SArgs                   (void);

protected:
    static void                         SWaitForStop            (const GpServiceStartFlags aFlags) noexcept;
    static void                         SInterruptWaitForStop   (void) noexcept;
    static int                          SStop                   (void) noexcept;
    static void                         SSystemSignalsHandler   (int aSignalId) noexcept;

protected:
    virtual GpArgBaseDesc::SP           CreateCmdLineArgsDesc   (void) = 0;
    virtual GpServiceCfgBaseDesc::SP    CreateServiceCfgDesc    (void) = 0;

    virtual GpServiceMainTask::SP       CreateMainTask          (std::string                    aName,
                                                                 GpItcPromise&&                 aStartPromise,
                                                                 const GpArgBaseDesc&           aCmdLineArgsDesc,
                                                                 const GpServiceCfgBaseDesc&    aServiceCfgDesc) = 0;

private:
    void                                Start                   (const size_t                   aArgc,
                                                                 char**                         aArgv,
                                                                 std::string                    aMainTaskName,
                                                                 const GpServiceStartFlags      aFlags,
                                                                 const GpLogConsumersFactory&   aLogConsumersFactory);
    void                                Stop                    (void);
    //ForkResT                          Fork                    (void);
    void                                ParseCmdLineArgs        (const size_t aArgc, char** aArgv);
    void                                ReadConfig              (const GpServiceStartFlags  aFlags);
    void                                SetSystemSignalsHandler (void);
    void                                StartLog                (const GpLogConsumersFactory& aConsumersFactory);
    void                                StartTaskScheduler      (const GpServiceStartFlags aFlags);
    void                                StopTaskScheduler       (void) noexcept;
    void                                StartMainTask           (std::string aName);

private:
    const std::string                   iName;
    GpArgBaseDesc::SP                   iCmdLineArgsDesc;
    GpServiceCfgBaseDesc::SP            iServiceCfgDesc;
    GpItcFuture::SP                     iMainTaskFuture;

    static std::mutex                   sServiceMutex;
    static GpService::SP                sService;
    static GpConditionVar               sServiceCondVar;
};

GpService::GpService (std::string aName) noexcept:
iName(std::move(aName))
{
}

}//GPlatform
