#include <GpService/ArgParser/GpServiceArgBaseDesc.hpp>
#include <GpCore2/GpReflection/GpReflectManager.hpp>
#include <GpCore2/GpReflection/GpReflectPropUtils.hpp>

namespace GPlatform {

REFLECT_IMPLEMENT(GpServiceArgBaseDesc, GP_MODULE_UUID)

GpServiceArgBaseDesc::GpServiceArgBaseDesc (void) noexcept
{
}

GpServiceArgBaseDesc::GpServiceArgBaseDesc (const GpServiceArgBaseDesc& aDesc):
GpReflectObject{aDesc},
background    {GpReflectUtils::SCopyValue(aDesc.background)},
log_level     {GpReflectUtils::SCopyValue(aDesc.log_level)},
cfg_file      {GpReflectUtils::SCopyValue(aDesc.cfg_file)},
resources_path{GpReflectUtils::SCopyValue(aDesc.resources_path)}
{
}

GpServiceArgBaseDesc::GpServiceArgBaseDesc (GpServiceArgBaseDesc&& aDesc) noexcept:
GpReflectObject{std::move(aDesc)},
background    {std::move(aDesc.background)},
log_level     {std::move(aDesc.log_level)},
cfg_file      {std::move(aDesc.cfg_file)},
resources_path{std::move(aDesc.resources_path)}
{
}

GpServiceArgBaseDesc::~GpServiceArgBaseDesc (void) noexcept
{
}

void    GpServiceArgBaseDesc::_SReflectCollectProps (GpReflectProp::SmallVecVal& aPropsOut)
{
    PROP(background);
    PROP(log_level);
    PROP(cfg_file);
    PROP(resources_path);
}

}// namespace GPlatform
