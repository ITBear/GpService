#pragma once

#include <GpService/GpServiceMainTask.hpp>

namespace GPlatform {

class GpServiceMainTaskFactory
{
public:
    CLASS_DD(GpServiceMainTaskFactory)

protected:
                                    GpServiceMainTaskFactory    (void) noexcept = default;

public:
    virtual                         ~GpServiceMainTaskFactory   (void) noexcept = default;

    virtual GpServiceMainTask::SP   NewInstance                 (void) const = 0;
};

}// namespace GPlatform
