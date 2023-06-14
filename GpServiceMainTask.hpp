#pragma once

#include "ArgParser/GpArgBaseDesc.hpp"
#include "GpServiceCfgBaseDesc.hpp"
#include "../GpLog/GpLogCore/Tasks/GpLogTaskFiberBase.hpp"
#include "../GpCore2/GpReflection/GpReflectManager.hpp"

namespace GPlatform {

class GP_SERVICE_API GpServiceMainTask: public GpLogTaskFiberBase
{
public:
    CLASS_REMOVE_CTRS_MOVE_COPY(GpServiceMainTask)
    CLASS_DD(GpServiceMainTask)

    using EventOptRefT      = std::optional<std::reference_wrapper<GpEvent>>;
    using StartItcPromiseT  = GpItcPromise<size_t>;
    using StartItcFutureT   = GpItcFuture<size_t>;
    using StartItcResultT   = GpItcResult<size_t>;

protected:
    inline                      GpServiceMainTask       (std::u8string                  aName,
                                                         StartItcPromiseT&&             aStartPromise,
                                                         const GpArgBaseDesc&           aCmdLineArgsDesc,
                                                         const GpServiceCfgBaseDesc&    aServiceCfgDesc) noexcept;
    virtual                     ~GpServiceMainTask      (void) noexcept override;

    const GpArgBaseDesc&        CmdLineArgs             (void) const noexcept {return iCmdLineArgsDesc;}

    template<typename T>
    const T&                    CmdLineArgsAs           (void) const;

    const GpServiceCfgBaseDesc& ServiceCfg              (void) const noexcept {return iServiceCfgDesc;}

    template<typename T>
    const T&                    ServiceCfgAs            (void) const;

protected:
    void                        CompleteStartPromise    (StartItcResultT::SP aResult) noexcept {iStartPromise.Complete(std::move(aResult));}

    virtual void                OnStart                 (void) override = 0;
    virtual GpTaskDoRes         OnStep                  (EventOptRefT aEvent) override = 0;
    virtual void                OnStop                  (void) noexcept override = 0;

private:
    StartItcPromiseT            iStartPromise;
    const GpArgBaseDesc&        iCmdLineArgsDesc;
    const GpServiceCfgBaseDesc& iServiceCfgDesc;
};

GpServiceMainTask::GpServiceMainTask
(
    std::u8string                   aName,
    StartItcPromiseT&&          aStartPromise,
    const GpArgBaseDesc&        aCmdLineArgsDesc,
    const GpServiceCfgBaseDesc& aServiceCfgDesc
) noexcept:
GpLogTaskFiberBase(std::move(aName)),
iStartPromise(std::move(aStartPromise)),
iCmdLineArgsDesc(aCmdLineArgsDesc),
iServiceCfgDesc(std::move(aServiceCfgDesc))
{
}

template<typename T>
const T&    GpServiceMainTask::CmdLineArgsAs (void) const
{
    const T& cmdLineArgsDesc = GpReflectManager::SCastRef<const T>(CmdLineArgs());
    return cmdLineArgsDesc;
}

template<typename T>
const T&    GpServiceMainTask::ServiceCfgAs (void) const
{
    const T& serviceCfgDesc = GpReflectManager::SCastRef<const T>(ServiceCfg());
    return serviceCfgDesc;
}

}//namespace GPlatform
