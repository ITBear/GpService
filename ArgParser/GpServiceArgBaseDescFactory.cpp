#include <GpService/ArgParser/GpServiceArgBaseDescFactory.hpp>

namespace GPlatform {

GpServiceArgBaseDescFactory::ResT   GpServiceArgBaseDescFactory::NewInstance
(
    [[maybe_unused]] const size_t       aArgc,
    [[maybe_unused]] const char* const  aArgv[]
) const
{
    return {MakeSP<GpServiceArgBaseDesc>(), false};
}

}// namespace GPlatform
