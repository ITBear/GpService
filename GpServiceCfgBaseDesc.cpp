#include "GpServiceCfgBaseDesc.hpp"

namespace GPlatform {

TYPE_STRUCT_IMPLEMENT(GpServiceCfgBaseDesc, GP_MODULE_UUID)

GpServiceCfgBaseDesc::GpServiceCfgBaseDesc (void) noexcept
{
}

GpServiceCfgBaseDesc::~GpServiceCfgBaseDesc (void) noexcept
{
}

void    GpServiceCfgBaseDesc::_SCollectStructProps (GpTypePropInfo::C::Vec::Val& aPropsOut)
{
    PROP(task_manager);
}

}//namespace GPlatform
