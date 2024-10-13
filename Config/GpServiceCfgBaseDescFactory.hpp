#pragma once

#include <GpService/Config/GpServiceCfgBaseDesc.hpp>

namespace GPlatform {

class GP_SERVICE_API GpServiceCfgBaseDescFactory
{
public:
    CLASS_DD(GpServiceCfgBaseDescFactory)

public:
                                        GpServiceCfgBaseDescFactory     (void) noexcept = default;
    virtual                             ~GpServiceCfgBaseDescFactory    (void) noexcept = default;

    virtual GpServiceCfgBaseDesc::SP    NewInstance                     (void) const;
};

}// namespace GPlatform
