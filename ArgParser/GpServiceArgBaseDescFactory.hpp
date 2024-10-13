#pragma once

#include <GpService/ArgParser/GpServiceArgBaseDesc.hpp>

namespace GPlatform {

class GP_SERVICE_API GpServiceArgBaseDescFactory
{
public:
    CLASS_DD(GpServiceArgBaseDescFactory)

    using ResT = std::tuple<GpServiceArgBaseDesc::SP, bool/*isEnableUnknownArguments*/>;

public:
                        GpServiceArgBaseDescFactory     (void) noexcept = default;
    virtual             ~GpServiceArgBaseDescFactory    (void) noexcept = default;

    virtual ResT        NewInstance                     (size_t             aArgc,
                                                         const char* const  aArgv[]) const;
};

}// namespace GPlatform
