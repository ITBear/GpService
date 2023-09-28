#include "GpServiceCfgTaskManagerDesc.hpp"

#include "../../GpCore2/GpReflection/GpReflectManager.hpp"

namespace GPlatform {

REFLECT_IMPLEMENT(GpServiceCfgTaskManagerDesc, GP_MODULE_UUID)

GpServiceCfgTaskManagerDesc::~GpServiceCfgTaskManagerDesc (void) noexcept
{
}

void    GpServiceCfgTaskManagerDesc::_SReflectCollectProps (GpReflectProp::C::Vec::Val& aPropsOut)
{
    PROP(fibers_max_cnt);
    PROP(fiber_stack_size);
    PROP(executors_cnt);
    PROP(tasks_max_cnt);
}

}//namespace GPlatform
