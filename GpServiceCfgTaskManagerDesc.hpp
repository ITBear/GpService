#pragma once

#include "GpService_global.hpp"
#include "../GpCore2/GpReflection/GpReflectObject.hpp"
#include "../GpCore2/GpReflection/GpReflectUtils.hpp"

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
    size_t              max_fibers_cnt      = 1024;
    size_byte_t         fiber_stack_size    = 128_kB;
    size_t              executors_cnt       = 1;
};

GpServiceCfgTaskManagerDesc::GpServiceCfgTaskManagerDesc (const GpServiceCfgTaskManagerDesc& aDesc):
GpReflectObject(aDesc),
max_fibers_cnt  (GpReflectUtils::SCopyValue(aDesc.max_fibers_cnt)),
fiber_stack_size(GpReflectUtils::SCopyValue(aDesc.fiber_stack_size)),
executors_cnt   (GpReflectUtils::SCopyValue(aDesc.executors_cnt))
{
}

GpServiceCfgTaskManagerDesc::GpServiceCfgTaskManagerDesc (GpServiceCfgTaskManagerDesc&& aDesc) noexcept:
GpReflectObject(std::move(aDesc)),
max_fibers_cnt  (std::move(aDesc.max_fibers_cnt)),
fiber_stack_size(std::move(aDesc.fiber_stack_size)),
executors_cnt   (std::move(aDesc.executors_cnt))
{
}

}//namespace GPlatform
