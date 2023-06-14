#pragma once

#include "GpArgBaseDesc.hpp"
#include "GpArgBaseDescFactory.hpp"

namespace boost::program_options
{
    class options_description;
}

namespace GPlatform {

class GP_SERVICE_API GpArgParser
{
public:
    CLASS_REMOVE_CTRS_DEFAULT_MOVE_COPY(GpArgParser)

    using OptDescT  = boost::program_options::options_description;

public:
    static GpArgBaseDesc::SP    SParse          (const size_t                   aArgc,
                                                 char**                         aArgv,
                                                 const GpArgBaseDescFactory&    aFactory,
                                                 std::u8string_view             aDescText);

private:
    static void                 SFillOptions    (OptDescT&              aOptDesc,
                                                 const GpArgBaseDesc&   aOut);
    static void                 SParseOptions   (const size_t       aArgc,
                                                 char**             aArgv,
                                                 const OptDescT&    aOptDesc,
                                                 GpArgBaseDesc&     aOut);
};

}//namespace GPlatform
