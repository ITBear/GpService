#include "GpArgBaseDesc.hpp"

namespace GPlatform {

TYPE_STRUCT_IMPLEMENT(GpArgBaseDesc, GP_MODULE_UUID)

GpArgBaseDesc::GpArgBaseDesc (void) noexcept
{
}

GpArgBaseDesc::~GpArgBaseDesc (void) noexcept
{
}

void    GpArgBaseDesc::_SCollectStructProps (GpTypePropInfo::C::Vec::Val& aPropsOut)
{
    PROP(run_background);
    PROP(cfg_file);
}

}//namespace GPlatform
