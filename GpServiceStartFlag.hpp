#pragma once

#include "GpService_global.hpp"
#include "../GpCore2/GpUtils/Types/Enums/GpEnum.hpp"
#include "../GpCore2/GpUtils/Types/Strings/GpStringOps.hpp"

namespace GPlatform {

GP_ENUM(GP_SERVICE_API, GpServiceStartFlag,
    CONFIG_REQUIRED
);

using GpServiceStartFlags = GpEnumFlagsST<GpServiceStartFlag>;

}//namespace GPlatform
