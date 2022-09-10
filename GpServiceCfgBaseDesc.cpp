#include "GpServiceCfgBaseDesc.hpp"

namespace GPlatform {

REFLECT_IMPLEMENT(GpServiceCfgBaseDesc, GP_MODULE_UUID)

GpServiceCfgBaseDesc::GpServiceCfgBaseDesc (void) noexcept
{
}

GpServiceCfgBaseDesc::GpServiceCfgBaseDesc (const GpServiceCfgBaseDesc& aDesc):
GpReflectObject(aDesc),
log(aDesc.log),//TODO: move
task_manager(aDesc.task_manager)
{
}

GpServiceCfgBaseDesc::GpServiceCfgBaseDesc (GpServiceCfgBaseDesc&& aDesc) noexcept:
GpReflectObject(std::move(aDesc)),
log(std::move(aDesc.log)),
task_manager(std::move(aDesc.task_manager))
{
}

GpServiceCfgBaseDesc::~GpServiceCfgBaseDesc (void) noexcept
{
}

void    GpServiceCfgBaseDesc::_SReflectCollectProps (GpReflectProp::C::Vec::Val& aPropsOut)
{
    PROP(log);
    PROP(task_manager);
}

}//namespace GPlatform
