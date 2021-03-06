#pragma once

#include "../GpService_global.hpp"

namespace GPlatform {

class GPSERVICE_API GpArgBaseDesc: public GpTypeStructBase
{
public:
    CLASS_DECLARE_DEFAULTS(GpArgBaseDesc)
    TYPE_STRUCT_DECLARE("56e26e3d-5f63-4dbc-b7c1-d23586c623ec"_sv)

public:
                            GpArgBaseDesc   (void) noexcept;
    virtual                 ~GpArgBaseDesc  (void) noexcept override;

    bool                    IsRunBackground (void) const noexcept {return run_background;}
    std::string_view        CfgFile         (void) const noexcept {return cfg_file;}

private:
    bool                    run_background  = false;
    std::string             cfg_file;
};

}//namespace GPlatform
