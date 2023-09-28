#pragma once

#include "GpServiceArgBaseDesc.hpp"

namespace GPlatform {

class GP_SERVICE_API GpServiceArgBaseDescFactory
{
public:
    CLASS_DD(GpServiceArgBaseDescFactory)

public:
                                        GpServiceArgBaseDescFactory     (void) noexcept = default;
    virtual                             ~GpServiceArgBaseDescFactory    (void) noexcept = default;

    virtual GpServiceArgBaseDesc::SP    NewInstance                     (const size_t   aArgc,
                                                                         char**         aArgv) const;
};

}//namespace GPlatform
