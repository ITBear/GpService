#include "GpServiceCfgTaskManagerDesc.hpp"

namespace GPlatform {

REFLECT_IMPLEMENT(GpServiceCfgTaskManagerDesc, GP_MODULE_UUID)

GpServiceCfgTaskManagerDesc::GpServiceCfgTaskManagerDesc (void) noexcept
{
}

GpServiceCfgTaskManagerDesc::GpServiceCfgTaskManagerDesc (const GpServiceCfgTaskManagerDesc& aDesc):
GpReflectObject(aDesc),
max_fibers_cnt(aDesc.max_fibers_cnt),
fiber_stack_size(aDesc.fiber_stack_size),
executors_cnt(aDesc.executors_cnt)
{
}

GpServiceCfgTaskManagerDesc::GpServiceCfgTaskManagerDesc (GpServiceCfgTaskManagerDesc&& aDesc) noexcept:
GpReflectObject(std::move(aDesc)),
max_fibers_cnt(std::move(aDesc.max_fibers_cnt)),
fiber_stack_size(std::move(aDesc.fiber_stack_size)),
executors_cnt(std::move(aDesc.executors_cnt))
{
}

GpServiceCfgTaskManagerDesc::~GpServiceCfgTaskManagerDesc (void) noexcept
{
}

void    GpServiceCfgTaskManagerDesc::_SReflectCollectProps (GpReflectProp::C::Vec::Val& aPropsOut)
{
    PROP(max_fibers_cnt);
    PROP(fiber_stack_size);
    PROP(executors_cnt);
}

}//namespace GPlatform
