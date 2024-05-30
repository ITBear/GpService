#include "GpServiceArgBaseDesc.hpp"

#include <GpCore2/GpReflection/GpReflectManager.hpp>
#include <GpCore2/GpReflection/GpReflectPropUtils.hpp>

namespace GPlatform {

REFLECT_IMPLEMENT(GpServiceArgBaseDesc, GP_MODULE_UUID)

GpServiceArgBaseDesc::~GpServiceArgBaseDesc (void) noexcept
{
}

void    GpServiceArgBaseDesc::_SReflectCollectProps (GpReflectProp::SmallVecVal& aPropsOut)
{
    PROP(background);
    PROP(cfg);
}

}// namespace GPlatform
