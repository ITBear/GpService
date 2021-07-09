#pragma once

#include "ArgParser/GpArgBaseDesc.hpp"
#include "GpServiceCfgBaseDesc.hpp"

namespace GPlatform {

class GPSERVICE_API GpServiceMainTask: public GpTaskFiberBase
{
public:
    CLASS_REMOVE_CTRS_MOVE_COPY(GpServiceMainTask)
    CLASS_DECLARE_DEFAULTS(GpServiceMainTask)

    using EventOptRefT  = std::optional<std::reference_wrapper<GpEvent>>;

protected:
                                GpServiceMainTask       (std::string_view           aName,
                                                         GpTaskFiberBarrier::SP     aTaskBarrierOnStart,
                                                         const GpArgBaseDesc&       aCmdLineArgsDesc,
                                                         GpServiceCfgBaseDesc::CSP  aServiceCfgDesc);
    virtual                     ~GpServiceMainTask      (void) noexcept override;

    const GpArgBaseDesc&        CmdLineArgs             (void) const noexcept {return iCmdLineArgsDesc;}
    const GpServiceCfgBaseDesc& ServiceCfg              (void) const noexcept {return iServiceCfgDesc.VC();}
    GpServiceCfgBaseDesc::CSP   ServiceCfgCSP           (void) const noexcept {return iServiceCfgDesc;}

protected:
    void                        ReleaseBarrierOnStart   (void);

private:
    GpTaskFiberBarrier::SP      iTaskBarrierOnStart;
    const GpArgBaseDesc&        iCmdLineArgsDesc;
    GpServiceCfgBaseDesc::CSP   iServiceCfgDesc;    
};

}//namespace GPlatform
