#pragma once

#include "../GpService_global.hpp"
#include "../../GpCore2/GpReflection/GpReflectObject.hpp"
#include "../../GpCore2/GpReflection/GpReflectUtils.hpp"

namespace GPlatform {

class GP_SERVICE_API GpArgBaseDesc: public GpReflectObject
{
public:
    CLASS_DD(GpArgBaseDesc)
    REFLECT_DECLARE(u8"56e26e3d-5f63-4dbc-b7c1-d23586c623ec"_uuid)

public:
                            GpArgBaseDesc   (void) noexcept;
    explicit                GpArgBaseDesc   (const GpArgBaseDesc& aDesc);
    explicit                GpArgBaseDesc   (GpArgBaseDesc&& aDesc) noexcept;
    virtual                 ~GpArgBaseDesc  (void) noexcept override;

    bool                    Background      (void) const noexcept {return background;}
    std::u8string_view      Cfg             (void) const noexcept {return cfg;}

private:
    bool                    background  = false;
    std::u8string           cfg;
};

}//namespace GPlatform
