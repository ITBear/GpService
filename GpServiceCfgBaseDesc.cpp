#include "GpServiceCfgBaseDesc.hpp"

namespace GPlatform {

TYPE_STRUCT_IMPLEMENT(GpServiceCfgBaseDesc, "7005f76c-e11c-441d-9d4f-dd9b7f0005de"_sv, GP_MODULE_UUID)

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
