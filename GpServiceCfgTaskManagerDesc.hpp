#pragma once

#include "GpService_global.hpp"

namespace GPlatform {

class GPSERVICE_API GpServiceCfgTaskManagerDesc: public GpTypeStructBase
{
public:
    CLASS_DECLARE_DEFAULTS(GpServiceCfgTaskManagerDesc)
    TYPE_STRUCT_DECLARE()

public:
                            GpServiceCfgTaskManagerDesc     (void) noexcept;
    virtual                 ~GpServiceCfgTaskManagerDesc    (void) noexcept override;

    count_t                 MaxFibersCnt                    (void) const noexcept {return max_fibers_cnt;}
    size_byte_t             FiberStackSize                  (void) const noexcept {return fiber_stack_size;}
    count_t                 ExecutorsCnt                    (void) const noexcept {return executors_cnt;}

private:
    count_t                 max_fibers_cnt      = 1024_cnt;
    size_byte_t             fiber_stack_size    = 128_kB;
    count_t                 executors_cnt       = 1_cnt;
};

}//namespace GPlatform
