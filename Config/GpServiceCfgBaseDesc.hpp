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
    inline explicit             GpServiceCfgBaseDesc    (const GpServiceCfgBaseDesc& aDesc);
    inline explicit             GpServiceCfgBaseDesc    (GpServiceCfgBaseDesc&& aDesc) noexcept;
    virtual                     ~GpServiceCfgBaseDesc   (void) noexcept override;

public:
    GpLogConfigDesc::SP         log;
    GpServiceCfgTaskManagerDesc task_manager;
};

GpServiceCfgBaseDesc::GpServiceCfgBaseDesc (const GpServiceCfgBaseDesc& aDesc):
GpReflectObject{aDesc},
log         {GpReflectUtils::SCopyValue(aDesc.log)},
task_manager{GpReflectUtils::SCopyValue(aDesc.task_manager)}
{
}

GpServiceCfgBaseDesc::GpServiceCfgBaseDesc (GpServiceCfgBaseDesc&& aDesc) noexcept:
GpReflectObject{std::move(aDesc)},
log         {std::move(aDesc.log)},
task_manager{std::move(aDesc.task_manager)}
{
}

}// namespace GPlatform
