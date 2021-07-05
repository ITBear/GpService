#include "GpServiceMainTask.hpp"

namespace GPlatform {

GpServiceMainTask::GpServiceMainTask
(
    std::string_view            aName,
    GpTaskFiberBarrier::SP      aTaskBarrierOnStart,
    const GpArgBaseDesc&        aCmdLineArgsDesc,
    GpServiceCfgBaseDesc::CSP   aServiceCfgDesc
):
GpTaskFiberBase(aName),
iTaskBarrierOnStart(std::move(aTaskBarrierOnStart)),
iCmdLineArgsDesc(aCmdLineArgsDesc),
iServiceCfgDesc(std::move(aServiceCfgDesc))
{
}

GpServiceMainTask::~GpServiceMainTask (void) noexcept
{   
    ReleaseBarrierOnStart();
}

void    GpServiceMainTask::ReleaseBarrierOnStart (void)
{
    if (iTaskBarrierOnStart.IsNotNULL())
    {
        iTaskBarrierOnStart->Release(std::nullopt);
        iTaskBarrierOnStart.Clear();
    }
}

}//namespace GPlatform
