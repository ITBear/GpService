#include <GpService/Config/GpServiceCfgBaseDesc.hpp>
#include <GpCore2/GpReflection/GpReflectManager.hpp>
#include <GpCore2/GpReflection/GpReflectPropUtils.hpp>

namespace GPlatform {

REFLECT_IMPLEMENT(GpServiceCfgBaseDesc, GP_MODULE_UUID)

GpServiceCfgBaseDesc::GpServiceCfgBaseDesc (const GpServiceCfgBaseDesc& aDesc):
GpReflectObject{aDesc},
log         {GpReflectUtils::SCopyValue(aDesc.log)},
task_manager{GpReflectUtils::SCopyValue(aDesc.task_manager)}
{
}

GpServiceCfgBaseDesc::GpServiceCfgBaseDesc (GpServiceCfgBaseDesc&& aDesc) noexcept:
GpReflectObject{std::move(aDesc)},
log         {std::move(aDesc.log)},
task_manager{std::move(aDesc.task_manager)}
{
}

GpServiceCfgBaseDesc::~GpServiceCfgBaseDesc (void) noexcept
{
}

void    GpServiceCfgBaseDesc::_SReflectCollectProps (GpReflectProp::SmallVecVal& aPropsOut)
{
    PROP(log);
    PROP(task_manager);
}

}// namespace GPlatform
