#include "GpServiceArgBaseDesc.hpp"
#include "../GpCore2/GpReflection/GpReflectManager.hpp"

namespace GPlatform {

REFLECT_IMPLEMENT(GpServiceArgBaseDesc, GP_MODULE_UUID)

GpServiceArgBaseDesc::~GpServiceArgBaseDesc (void) noexcept
{
}

void    GpServiceArgBaseDesc::_SReflectCollectProps (GpReflectProp::C::Vec::Val& aPropsOut)
{
    PROP(background);
    PROP(cfg);
}

}//namespace GPlatform
