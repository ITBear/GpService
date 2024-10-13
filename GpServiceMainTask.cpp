#include <GpService/GpServiceMainTask.hpp>
#include <GpService/GpService.hpp>

namespace GPlatform {

GpServiceMainTask::GpServiceMainTask
(
    const GpServiceArgBaseDesc& aServiceArgsDesc,
    const GpServiceCfgBaseDesc& aServiceCfgDesc
) noexcept:
iServiceArgsDesc{aServiceArgsDesc},
iServiceCfgDesc {std::move(aServiceCfgDesc)}
{
}

GpServiceMainTask::~GpServiceMainTask (void) noexcept
{
}

void    GpServiceMainTask::OnStart (void)
{
    // NOP
}

void    GpServiceMainTask::OnStop ([[maybe_unused]] StopExceptionsT& aStopExceptionsOut) noexcept
{
    GpService::SRequestStop();
}

}// namespace GPlatform
