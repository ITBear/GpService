#pragma once

#include "GpServiceCfgTaskManagerDesc.hpp"

namespace GPlatform {

class GPSERVICE_API GpServiceCfgBaseDesc: public GpTypeStructBase
{
public:
    CLASS_DECLARE_DEFAULTS(GpServiceCfgBaseDesc)
    TYPE_STRUCT_DECLARE("7005f76c-e11c-441d-9d4f-dd9b7f0005de"_sv)

public:
                                        GpServiceCfgBaseDesc    (void) noexcept;
    virtual                             ~GpServiceCfgBaseDesc   (void) noexcept override;

    const GpLogConfigDesc&              Log                     (void) const noexcept {return log;}
    const GpServiceCfgTaskManagerDesc&  TaskManager             (void) const noexcept {return task_manager;}

private:
    GpLogConfigDesc                     log;
    GpServiceCfgTaskManagerDesc         task_manager;
};

}//namespace GPlatform
