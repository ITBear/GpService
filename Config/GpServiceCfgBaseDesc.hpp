#pragma once

#include <GpService/Config/GpServiceCfgTaskManagerDesc.hpp>
#include <GpLog/GpLogCore/Config/GpLogConfigDesc.hpp>

namespace GPlatform {

class GP_SERVICE_API GpServiceCfgBaseDesc: public GpReflectObject
{
public:
    CLASS_DD(GpServiceCfgBaseDesc)
    REFLECT_DECLARE("7005f76c-e11c-441d-9d4f-dd9b7f0005de"_uuid)

public:
                                GpServiceCfgBaseDesc    (void) noexcept = default;
    explicit                    GpServiceCfgBaseDesc    (const GpServiceCfgBaseDesc& aDesc);
    explicit                    GpServiceCfgBaseDesc    (GpServiceCfgBaseDesc&& aDesc) noexcept;
    virtual                     ~GpServiceCfgBaseDesc   (void) noexcept override;

public:
    GpLogConfigDesc::SP         log;
    GpServiceCfgTaskManagerDesc task_manager;
};

}// namespace GPlatform
