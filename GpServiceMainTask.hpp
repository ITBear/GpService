#pragma once

#include "ArgParser/GpArgBaseDesc.hpp"
#include "GpServiceCfgBaseDesc.hpp"

namespace GPlatform {

class GP_SERVICE_API GpServiceMainTask: public GpLogTaskFiberBase
{
public:
    CLASS_REMOVE_CTRS_MOVE_COPY(GpServiceMainTask)
    CLASS_DECLARE_DEFAULTS(GpServiceMainTask)

    using EventOptRefT  = std::optional<std::reference_wrapper<GpEvent>>;

protected:
    inline                      GpServiceMainTask       (std::string                    aName,
                                                         GpItcPromise&&                 aStartPromise,
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
    inline void                 CompleteStartPromise    (GpItcResult::SP aResult) noexcept {iStartPromise.Complete(std::move(aResult));}

    virtual void                OnStart                 (void) override = 0;
    virtual GpTaskDoRes         OnStep                  (EventOptRefT aEvent) override = 0;
    virtual void                OnStop                  (void) noexcept override = 0;

private:
    GpItcPromise                iStartPromise;
    const GpArgBaseDesc&        iCmdLineArgsDesc;
    const GpServiceCfgBaseDesc& iServiceCfgDesc;
};

GpServiceMainTask::GpServiceMainTask
(
    std::string                 aName,
    GpItcPromise&&              aStartPromise,
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
