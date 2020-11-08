#pragma once

#include "GpServiceCfgTaskManagerDesc.hpp"

namespace GPlatform {

class GPSERVICE_API GpServiceCfgBaseDesc: public GpTypeStructBase
{
public:
    CLASS_DECLARE_DEFAULTS(GpServiceCfgBaseDesc)
    TYPE_STRUCT_DECLARE()

public:
                                        GpServiceCfgBaseDesc    (void) noexcept;
    virtual                             ~GpServiceCfgBaseDesc   (void) noexcept override;

    const GpServiceCfgTaskManagerDesc&  TaskManager             (void) const noexcept {return task_manager;}

private:
    GpServiceCfgTaskManagerDesc         task_manager;
};

}//namespace GPlatform
