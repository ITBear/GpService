#pragma once

#include "GpArgBaseDesc.hpp"

namespace GPlatform {

class GpArgBaseDescFactory
{
public:
    CLASS_DD(GpArgBaseDescFactory)

protected:
                                GpArgBaseDescFactory    (void) noexcept = default;

public:
    virtual                     ~GpArgBaseDescFactory   (void) noexcept = default;

    virtual GpArgBaseDesc::SP   NewInstance             (const size_t   aArgc,
                                                         char**         aArgv) const = 0;
};

}//namespace GPlatform
