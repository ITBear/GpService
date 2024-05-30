#include "GpServiceArgBaseDescFactory.hpp"

namespace GPlatform {

GpServiceArgBaseDesc::SP    GpServiceArgBaseDescFactory::NewInstance
(
    const size_t    /*aArgc*/,
    char**          /*aArgv*/
) const
{
    return MakeSP<GpServiceArgBaseDesc>();
}

}// namespace GPlatform
