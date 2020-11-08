#pragma once

#include "GpService_global.hpp"

namespace GPlatform {

class GpService;

class GPSERVICE_API GpServiceFactory
{
public:
    CLASS_REMOVE_CTRS_EXCEPT_DEFAULT(GpServiceFactory)
    CLASS_DECLARE_DEFAULTS(GpServiceFactory)

protected:
                                GpServiceFactory    (void) noexcept {}

public:
    virtual                     ~GpServiceFactory   (void) noexcept {}

    virtual GpSP<GpService>     NewInstance         (void) const = 0;
};

}//namespace GPlatform
