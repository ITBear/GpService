#pragma once

#include "../GpCore2/GpCore.hpp"
#include "../GpJson/GpJson.hpp"
#include "../GpLog/GpLog.hpp"

#if defined(GPSERVICE_LIBRARY)
    #define GPSERVICE_API GP_DECL_EXPORT
#else
    #define GPSERVICE_API GP_DECL_IMPORT
#endif
