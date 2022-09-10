#pragma once

#include "GpService_global.hpp"

namespace GPlatform {

GP_ENUM(GP_SERVICE_API, GpServiceStartFlag,
    CONFIG_REQUIRED
);

using GpServiceStartFlags = GpEnumFlagsST<GpServiceStartFlag>;

}//namespace GPlatform
