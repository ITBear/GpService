#pragma once

#include "GpService_global.hpp"

namespace GPlatform {

class GP_SERVICE_API GpServiceCfgTaskManagerDesc: public GpReflectObject
{
public:
    CLASS_DECLARE_DEFAULTS(GpServiceCfgTaskManagerDesc)
    REFLECT_DECLARE("53538c79-f429-4ac3-8c13-b1a2dab02f99"_uuid)

public:
                            GpServiceCfgTaskManagerDesc     (void) noexcept;
    explicit                GpServiceCfgTaskManagerDesc     (const GpServiceCfgTaskManagerDesc& aDesc);
    explicit                GpServiceCfgTaskManagerDesc     (GpServiceCfgTaskManagerDesc&& aDesc) noexcept;
    virtual                 ~GpServiceCfgTaskManagerDesc    (void) noexcept override;

    size_t                  MaxFibersCnt                    (void) const noexcept {return max_fibers_cnt;}
    size_byte_t             FiberStackSize                  (void) const noexcept {return fiber_stack_size;}
    size_t                  ExecutorsCnt                    (void) const noexcept {return executors_cnt;}

private:
    size_t                  max_fibers_cnt      = 1024;
    size_byte_t             fiber_stack_size    = 128_kB;
    size_t                  executors_cnt       = 1;
};

}//namespace GPlatform
