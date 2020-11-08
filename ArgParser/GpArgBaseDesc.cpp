#include "GpArgBaseDesc.hpp"

namespace GPlatform {

TYPE_STRUCT_IMPLEMENT(GpArgBaseDesc, "56e26e3d-5f63-4dbc-b7c1-d23586c623ec"_sv, GP_MODULE_UUID)

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
