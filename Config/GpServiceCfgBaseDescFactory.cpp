#include "GpServiceCfgBaseDescFactory.hpp"

namespace GPlatform {

GpServiceCfgBaseDesc::SP    GpServiceCfgBaseDescFactory::NewInstance (void) const
{
    return MakeSP<GpServiceCfgBaseDesc>();
}

}//namespace GPlatform
