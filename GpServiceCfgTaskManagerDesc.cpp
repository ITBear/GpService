#include "GpServiceCfgTaskManagerDesc.hpp"

namespace GPlatform {

TYPE_STRUCT_IMPLEMENT(GpServiceCfgTaskManagerDesc, "53538c79-f429-4ac3-8c13-b1a2dab02f99"_sv, GP_MODULE_UUID)

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
