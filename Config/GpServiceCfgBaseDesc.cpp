#include "GpServiceCfgBaseDesc.hpp"
#include "../../GpCore2/GpReflection/GpReflectManager.hpp"

namespace GPlatform {

REFLECT_IMPLEMENT(GpServiceCfgBaseDesc, GP_MODULE_UUID)

GpServiceCfgBaseDesc::~GpServiceCfgBaseDesc (void) noexcept
{
}

void    GpServiceCfgBaseDesc::_SReflectCollectProps (GpReflectProp::C::Vec::Val& aPropsOut)
{
    PROP(log);
    PROP(task_manager);
}

}//namespace GPlatform
