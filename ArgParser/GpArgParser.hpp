#pragma once

#include "GpArgBaseDesc.hpp"

namespace boost::program_options
{
    class options_description;
}

namespace GPlatform {

class GPSERVICE_API GpArgParser
{
public:
    CLASS_REMOVE_CTRS(GpArgParser)

    using OptDescT  = boost::program_options::options_description;

public:
    static void                 SParse          (const size_t       aArgc,
                                                 char**             aArgv,
                                                 GpArgBaseDesc&     aOut,
                                                 std::string_view   aDescText);

private:
    static void                 SFillOptions    (OptDescT&              aOptDesc,
                                                 const GpArgBaseDesc&   aOut);
    static void                 SParseOptions   (const size_t       aArgc,
                                                 char**             aArgv,
                                                 const OptDescT&    aOptDesc,
                                                 GpArgBaseDesc&     aOut);
};

}//namespace GPlatform
