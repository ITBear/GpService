#include "GpServiceMainTask.hpp"

namespace GPlatform {

GpServiceMainTask::GpServiceMainTask (const GpArgBaseDesc&      aCmdLineArgsDesc,
                                      GpServiceCfgBaseDesc::CSP aServiceCfgDesc) noexcept:
iCmdLineArgsDesc(aCmdLineArgsDesc),
iServiceCfgDesc(std::move(aServiceCfgDesc))
{
}

GpServiceMainTask::~GpServiceMainTask (void) noexcept
{
}

}//namespace GPlatform
