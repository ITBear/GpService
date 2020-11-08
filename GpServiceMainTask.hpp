#pragma once

#include "ArgParser/GpArgBaseDesc.hpp"
#include "GpServiceCfgBaseDesc.hpp"

namespace GPlatform {

class GPSERVICE_API GpServiceMainTask: public GpTaskBase
{
public:
    CLASS_REMOVE_CTRS_EXCEPT_DEFAULT(GpServiceMainTask)
    CLASS_DECLARE_DEFAULTS(GpServiceMainTask)

    using EventOptRefT  = std::optional<std::reference_wrapper<GpEvent>>;

protected:
                                GpServiceMainTask       (const GpArgBaseDesc&       aCmdLineArgsDesc,
                                                         GpServiceCfgBaseDesc::CSP  aServiceCfgDesc) noexcept;
    virtual                     ~GpServiceMainTask      (void) noexcept override;

    const GpArgBaseDesc&        CmdLineArgs             (void) const noexcept {return iCmdLineArgsDesc;}
    const GpServiceCfgBaseDesc& ServiceCfg              (void) const noexcept {return iServiceCfgDesc.VC();}
    GpServiceCfgBaseDesc::CSP   ServiceCfgCSP           (void) const noexcept {return iServiceCfgDesc;}

private:
    const GpArgBaseDesc&        iCmdLineArgsDesc;
    GpServiceCfgBaseDesc::CSP   iServiceCfgDesc;
};

}//namespace GPlatform
