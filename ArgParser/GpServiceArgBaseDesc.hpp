#pragma once

#include <GpService/GpService_global.hpp>
#include <GpCore2/GpReflection/GpReflectObject.hpp>
#include <GpCore2/GpReflection/GpReflectUtils.hpp>
#include <GpLog/GpLogCore/GpLogLevel.hpp>

namespace GPlatform {

class GP_SERVICE_API GpServiceArgBaseDesc: public GpReflectObject
{
public:
    CLASS_DD(GpServiceArgBaseDesc)
    REFLECT_DECLARE("56e26e3d-5f63-4dbc-b7c1-d23586c623ec"_uuid)

public:
                        GpServiceArgBaseDesc    (void) noexcept;
                        GpServiceArgBaseDesc    (const GpServiceArgBaseDesc& aDesc);
                        GpServiceArgBaseDesc    (GpServiceArgBaseDesc&& aDesc) noexcept;
    virtual             ~GpServiceArgBaseDesc   (void) noexcept override;

    bool                Background              (void) const noexcept {return background;}
    GpLogLevel::EnumT   LogLevel                (void) const noexcept {return log_level;}
    std::string_view    CfgFile                 (void) const noexcept {return cfg_file;}

private:
    bool                background  = false;
    GpLogLevel          log_level   = GpLogLevel::L_CRITICAL_ERROR;
    std::string         cfg_file;
};

}// namespace GPlatform
