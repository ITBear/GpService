#pragma once

#include <GpService/ArgParser/GpServiceArgBaseDesc.hpp>
#include <GpService/ArgParser/GpServiceArgBaseDescFactory.hpp>
#include <GpCore2/GpUtils/Other/ArgParser/GpArgParser.hpp>

namespace GPlatform {

class GP_SERVICE_API GpServiceArgParser
{
public:
    CLASS_REMOVE_CTRS_DEFAULT_MOVE_COPY(GpServiceArgParser)

    using ResT = std::tuple<GpServiceArgBaseDesc::SP, GpArgParserRes::SP>;

public:
    static ResT                 SParse          (size_t                             aArgc,
                                                 const char* const                  aArgv[],
                                                 const GpServiceArgBaseDescFactory& aFactory);

private:
    static void                 SInitArgParser  (const GpServiceArgBaseDesc&    aArgBaseDesc,
                                                 GpArgParser&                   aArgParser);
    static GpArgParserRes::SP   SParse          (size_t                 aArgc,
                                                 const char* const      aArgv[],
                                                 const GpArgParser&     aArgParser,
                                                 GpServiceArgBaseDesc&  aArgBaseDescOut);
};

}// namespace GPlatform
