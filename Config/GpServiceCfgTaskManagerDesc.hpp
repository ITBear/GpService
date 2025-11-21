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
    virtual     ~GpServiceCfgTaskManagerDesc    (void) noexcept override;

public:
    u_int_32    fibers_max_cnt      = 4096;
    size_byte_t fiber_stack_size    = 1024_kB;
    u_int_32    executors_cnt       = 4;
    u_int_32    tasks_max_cnt       = 4096;
};

}// namespace GPlatform
