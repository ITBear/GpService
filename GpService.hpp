#pragma once

#include "GpServiceMainTask.hpp"
#include "GpServiceFactory.hpp"
#include "GpServiceStartFlag.hpp"
#include "GpServiceCfgBaseDesc.hpp"
#include "ArgParser/GpArgBaseDescFactory.hpp"
#include "../GpCore2/GpTasks/GpTaskFactory.hpp"
#include "../GpCore2/GpUtils/SyncPrimitives/GpConditionVarFlag.hpp"

namespace GPlatform {

class GpLogConsumersFactory;

class GP_SERVICE_API GpService
{
public:
    CLASS_REMOVE_CTRS_DEFAULT_MOVE_COPY(GpService)
    CLASS_DD(GpService)

    enum class ForkResT
    {
        CHILD,
        PARENT
    };

    using StartItcPromiseT  = GpItcPromise<size_t>;
    using StartItcFutureT   = GpItcFuture<size_t>;
    using StartItcResultT   = GpItcResult<size_t>;

protected:
    inline                              GpService                   (std::u8string aName) noexcept;

public:
    virtual                             ~GpService                  (void) noexcept;

    [[nodiscard]] static int            SRun                        (const size_t                       aArgc,
                                                                     char**                             aArgv,
                                                                     const GpServiceFactory::SP         aServiceFactory,
                                                                     const GpServiceStartFlags          aFlags,
                                                                     const GpLogConsumersFactory&       aLogConsumersFactory,
                                                                     const GpTaskFactory::C::Vec::SP&   aTasks);
    static void                         SRequestStop                (void) noexcept;
    static GpArgBaseDesc::C::Opt::CRef  SArgs                       (void);

protected:
    static void                         SWaitForStop                (const GpServiceStartFlags aFlags) noexcept;
    static void                         SInterruptWaitForStop       (void) noexcept;
    static int                          SStop                       (void) noexcept;

    static void                         SSystemSignalsHandler       (int aSignalId) noexcept;
    static void                         SPrintStacktrace            (void);

protected:
    virtual GpArgBaseDescFactory::SP    CreateCmdLineArgsDescFactory(void) = 0;
    virtual GpServiceCfgBaseDesc::SP    CreateServiceCfgDesc        (void) = 0;

    virtual GpServiceMainTask::SP       CreateMainTask              (StartItcPromiseT&&             aStartPromise,
                                                                     const GpArgBaseDesc&           aCmdLineArgsDesc,
                                                                     const GpServiceCfgBaseDesc&    aServiceCfgDesc) = 0;

private:
    void                                Start                       (const size_t                   aArgc,
                                                                     char**                         aArgv,
                                                                     const GpServiceStartFlags      aFlags,
                                                                     const GpLogConsumersFactory&   aLogConsumersFactory);
    void                                Stop                        (void);
    //ForkResT                          Fork                        (void);
    void                                ParseCmdLineArgs            (const size_t aArgc, char** aArgv);
    void                                ReadConfig                  (const GpServiceStartFlags  aFlags);
    void                                SetSystemSignalsHandler     (void);
    void                                StartLog                    (const GpLogConsumersFactory&   aConsumersFactory,
                                                                     const GpServiceStartFlags      aFlags);
    void                                StartTaskScheduler          (const GpServiceStartFlags aFlags);
    void                                StopTaskScheduler           (void) noexcept;
    void                                StartMainTask               (void);

    static void                         SStartTasks                 (const GpTaskFactory::C::Vec::SP& aTasks);

private:
    const std::u8string                 iName;
    GpArgBaseDesc::SP                   iCmdLineArgsDesc;
    GpServiceCfgBaseDesc::SP            iServiceCfgDesc;
    StartItcFutureT::SP                 iMainTaskFuture;

    static std::mutex                   sServiceMutex;
    static GpService::SP                sService;
    static GpConditionVarFlag           sServiceCondVar;
};

GpService::GpService (std::u8string aName) noexcept:
iName(std::move(aName))
{
}

}//GPlatform
