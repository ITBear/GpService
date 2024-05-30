#pragma once

#include "GpServiceArgBaseDesc.hpp"
#include "GpServiceArgBaseDescFactory.hpp"

namespace boost::program_options
{
    class options_description;
}

namespace GPlatform {

class GP_SERVICE_API GpServiceArgParser
{
public:
    CLASS_REMOVE_CTRS_DEFAULT_MOVE_COPY(GpServiceArgParser)

    using OptDescT  = boost::program_options::options_description;

public:
    static GpServiceArgBaseDesc::SP SParse          (size_t                             aArgc,
                                                     char**                             aArgv,
                                                     const GpServiceArgBaseDescFactory& aFactory,
                                                     std::string_view                   aDescText);

private:
    static void                     SFillOptions    (OptDescT&                      aOptDesc,
                                                     const GpServiceArgBaseDesc&    aOut);
    static void                     SParseOptions   (size_t                 aArgc,
                                                     char**                 aArgv,
                                                     const OptDescT&        aOptDesc,
                                                     GpServiceArgBaseDesc&  aOut);
};

}// namespace GPlatform
