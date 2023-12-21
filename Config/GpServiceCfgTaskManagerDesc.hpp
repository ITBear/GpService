#pragma once

#include "../GpService_global.hpp"

#include <GpCore2/GpReflection/GpReflectObject.hpp>
#include <GpCore2/GpReflection/GpReflectUtils.hpp>

namespace GPlatform {

class GP_SERVICE_API GpServiceCfgTaskManagerDesc: public GpReflectObject
{
public:
    CLASS_DD(GpServiceCfgTaskManagerDesc)
    REFLECT_DECLARE(u8"53538c79-f429-4ac3-8c13-b1a2dab02f99"_uuid)

public:
                        GpServiceCfgTaskManagerDesc     (void) noexcept = default;
    inline              GpServiceCfgTaskManagerDesc     (const GpServiceCfgTaskManagerDesc& aDesc);
    inline              GpServiceCfgTaskManagerDesc     (GpServiceCfgTaskManagerDesc&& aDesc) noexcept;
    virtual             ~GpServiceCfgTaskManagerDesc    (void) noexcept override;

public:
    size_t              fibers_max_cnt      = 1024;
    size_byte_t         fiber_stack_size    = 128_kB;
    size_t              executors_cnt       = 1;
    size_t              tasks_max_cnt       = 1024;
};

GpServiceCfgTaskManagerDesc::GpServiceCfgTaskManagerDesc (const GpServiceCfgTaskManagerDesc& aDesc):
GpReflectObject(aDesc),
fibers_max_cnt  (GpReflectUtils::SCopyValue(aDesc.fibers_max_cnt)),
fiber_stack_size(GpReflectUtils::SCopyValue(aDesc.fiber_stack_size)),
executors_cnt   (GpReflectUtils::SCopyValue(aDesc.executors_cnt)),
tasks_max_cnt   (GpReflectUtils::SCopyValue(aDesc.tasks_max_cnt))
{
}

GpServiceCfgTaskManagerDesc::GpServiceCfgTaskManagerDesc (GpServiceCfgTaskManagerDesc&& aDesc) noexcept:
GpReflectObject(std::move(aDesc)),
fibers_max_cnt  (std::move(aDesc.fibers_max_cnt)),
fiber_stack_size(std::move(aDesc.fiber_stack_size)),
executors_cnt   (std::move(aDesc.executors_cnt)),
tasks_max_cnt   (std::move(aDesc.tasks_max_cnt))
{
}

}//namespace GPlatform
