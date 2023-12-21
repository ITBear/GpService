#pragma once

#include "../GpService_global.hpp"

#include <GpCore2/GpReflection/GpReflectObject.hpp>
#include <GpCore2/GpReflection/GpReflectUtils.hpp>

namespace GPlatform {

class GP_SERVICE_API GpServiceArgBaseDesc: public GpReflectObject
{
public:
    CLASS_DD(GpServiceArgBaseDesc)
    REFLECT_DECLARE(u8"56e26e3d-5f63-4dbc-b7c1-d23586c623ec"_uuid)

public:
    inline                  GpServiceArgBaseDesc    (void) noexcept;
    inline                  GpServiceArgBaseDesc    (const GpServiceArgBaseDesc& aDesc);
    inline                  GpServiceArgBaseDesc    (GpServiceArgBaseDesc&& aDesc) noexcept;
    virtual                 ~GpServiceArgBaseDesc   (void) noexcept override;

    bool                    Background              (void) const noexcept {return background;}
    std::u8string_view      Cfg                     (void) const noexcept {return cfg;}

private:
    bool                    background  = false;
    std::u8string           cfg;
};

GpServiceArgBaseDesc::GpServiceArgBaseDesc (void) noexcept
{
}

GpServiceArgBaseDesc::GpServiceArgBaseDesc (const GpServiceArgBaseDesc& aDesc):
GpReflectObject(aDesc),
background(GpReflectUtils::SCopyValue(aDesc.background)),
cfg       (GpReflectUtils::SCopyValue(aDesc.cfg))
{
}

GpServiceArgBaseDesc::GpServiceArgBaseDesc (GpServiceArgBaseDesc&& aDesc) noexcept:
GpReflectObject(std::move(aDesc)),
background(std::move(aDesc.background)),
cfg       (std::move(aDesc.cfg))
{
}

}//namespace GPlatform
