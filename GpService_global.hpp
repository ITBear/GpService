#pragma once

#include <GpCore2/GpUtils/Macro/GpMacroImportExport.hpp>

#if defined(GP_SERVICE_LIBRARY)
    #define GP_SERVICE_API GP_DECL_EXPORT
#else
    #define GP_SERVICE_API GP_DECL_IMPORT
#endif
