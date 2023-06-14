#include "GpArgBaseDesc.hpp"
#include "../GpCore2/GpReflection/GpReflectManager.hpp"

namespace GPlatform {

REFLECT_IMPLEMENT(GpArgBaseDesc, GP_MODULE_UUID)

GpArgBaseDesc::GpArgBaseDesc (void) noexcept
{
}

GpArgBaseDesc::GpArgBaseDesc (const GpArgBaseDesc& aDesc):
GpReflectObject(aDesc),
background(aDesc.background),
cfg(aDesc.cfg)
{
}

GpArgBaseDesc::GpArgBaseDesc (GpArgBaseDesc&& aDesc) noexcept:
GpReflectObject(std::move(aDesc)),
background(std::move(aDesc.background)),
cfg(std::move(aDesc.cfg))
{
}

GpArgBaseDesc::~GpArgBaseDesc (void) noexcept
{
}

void    GpArgBaseDesc::_SReflectCollectProps (GpReflectProp::C::Vec::Val& aPropsOut)
{
    PROP(background);
    PROP(cfg);
}

}//namespace GPlatform
