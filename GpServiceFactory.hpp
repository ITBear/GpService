#pragma once

#include "GpService_global.hpp"

namespace GPlatform {

class GpService;

class GP_SERVICE_API GpServiceFactory
{
public:
    CLASS_REMOVE_CTRS_MOVE_COPY(GpServiceFactory)
    CLASS_DECLARE_DEFAULTS(GpServiceFactory)

protected:
                                GpServiceFactory    (void) noexcept {}

public:
    virtual                     ~GpServiceFactory   (void) noexcept {}

    virtual GpSP<GpService>     NewInstance         (void) const = 0;
};

}//namespace GPlatform
