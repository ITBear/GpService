#include <GpService/Config/GpServiceCfgTaskManagerDesc.hpp>
#include <GpCore2/GpReflection/GpReflectManager.hpp>
#include <GpCore2/GpReflection/GpReflectPropUtils.hpp>

namespace GPlatform {

REFLECT_IMPLEMENT(GpServiceCfgTaskManagerDesc, GP_MODULE_UUID)

GpServiceCfgTaskManagerDesc::GpServiceCfgTaskManagerDesc (const GpServiceCfgTaskManagerDesc& aDesc):
GpReflectObject{aDesc},
fibers_max_cnt  {GpReflectUtils::SCopyValue(aDesc.fibers_max_cnt)},
fiber_stack_size{GpReflectUtils::SCopyValue(aDesc.fiber_stack_size)},
executors_cnt   {GpReflectUtils::SCopyValue(aDesc.executors_cnt)},
tasks_max_cnt   {GpReflectUtils::SCopyValue(aDesc.tasks_max_cnt)}
{
}

GpServiceCfgTaskManagerDesc::GpServiceCfgTaskManagerDesc (GpServiceCfgTaskManagerDesc&& aDesc) noexcept:
GpReflectObject{std::move(aDesc)},
fibers_max_cnt  {std::move(aDesc.fibers_max_cnt)},
fiber_stack_size{std::move(aDesc.fiber_stack_size)},
executors_cnt   {std::move(aDesc.executors_cnt)},
tasks_max_cnt   {std::move(aDesc.tasks_max_cnt)}
{
}

GpServiceCfgTaskManagerDesc::~GpServiceCfgTaskManagerDesc (void) noexcept
{
}

void    GpServiceCfgTaskManagerDesc::_SReflectCollectProps (GpReflectProp::SmallVecVal& aPropsOut)
{
    PROP(fibers_max_cnt);
    PROP(fiber_stack_size);
    PROP(executors_cnt);
    PROP(tasks_max_cnt);
}

}// namespace GPlatform
