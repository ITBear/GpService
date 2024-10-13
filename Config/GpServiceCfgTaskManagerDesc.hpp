#pragma once

#include <GpService/GpService_global.hpp>

#include <GpCore2/GpReflection/GpReflectObject.hpp>
#include <GpCore2/GpReflection/GpReflectUtils.hpp>
#include <GpCore2/GpUtils/Types/Units/Other/size_byte_t.hpp>

namespace GPlatform {

class GP_SERVICE_API GpServiceCfgTaskManagerDesc: public GpReflectObject
{
public:
    CLASS_DD(GpServiceCfgTaskManagerDesc)
    REFLECT_DECLARE("53538c79-f429-4ac3-8c13-b1a2dab02f99"_uuid)

public:
                    GpServiceCfgTaskManagerDesc     (void) noexcept = default;
                    GpServiceCfgTaskManagerDesc     (const GpServiceCfgTaskManagerDesc& aDesc);
                    GpServiceCfgTaskManagerDesc     (GpServiceCfgTaskManagerDesc&& aDesc) noexcept;
    virtual         ~GpServiceCfgTaskManagerDesc    (void) noexcept override;

public:
    size_t          fibers_max_cnt      = 1024;
    size_byte_t     fiber_stack_size    = 256_kB;
    size_t          executors_cnt       = 1;
    size_t          tasks_max_cnt       = 1024;
};

}// namespace GPlatform
