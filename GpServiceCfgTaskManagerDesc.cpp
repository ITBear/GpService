#include "GpServiceCfgTaskManagerDesc.hpp"

namespace GPlatform {

TYPE_STRUCT_IMPLEMENT(GpServiceCfgTaskManagerDesc, GP_MODULE_UUID)

GpServiceCfgTaskManagerDesc::GpServiceCfgTaskManagerDesc (void) noexcept
{
}

GpServiceCfgTaskManagerDesc::~GpServiceCfgTaskManagerDesc (void) noexcept
{
}

void    GpServiceCfgTaskManagerDesc::_SCollectStructProps (GpTypePropInfo::C::Vec::Val& aPropsOut)
{
    PROP(max_fibers_cnt);
    PROP(fiber_stack_size);
    PROP(executors_cnt);
}

}//namespace GPlatform
