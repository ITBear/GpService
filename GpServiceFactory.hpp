#pragma once

#include "GpService_global.hpp"
#include "../GpCore2/GpUtils/Macro/GpMacroClass.hpp"
#include "../GpCore2/GpUtils/Types/Containers/GpContainersT.hpp"

namespace GPlatform {

class GpService;

class GP_SERVICE_API GpServiceFactory
{
public:
    CLASS_REMOVE_CTRS_MOVE_COPY(GpServiceFactory)
    CLASS_DD(GpServiceFactory)

protected:
                                GpServiceFactory    (void) noexcept = default;

public:
    virtual                     ~GpServiceFactory   (void) noexcept = default;

    virtual GpSP<GpService>     NewInstance         (void) const = 0;
};

}//namespace GPlatform
