#pragma once

#include "GpServiceMainTask.hpp"

namespace GPlatform {

class GpServiceMainTaskFactory
{
public:
    CLASS_DD(GpServiceMainTaskFactory)

protected:
                                    GpServiceMainTaskFactory    (void) noexcept = default;

public:
    virtual                         ~GpServiceMainTaskFactory   (void) noexcept = default;

    virtual GpServiceMainTask::SP   NewInstance                 (const GpServiceArgBaseDesc& aServiceArgsDesc,
                                                                 const GpServiceCfgBaseDesc& aServiceCfgDesc) const = 0;
};

}//namespace GPlatform
