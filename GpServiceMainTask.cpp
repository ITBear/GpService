#include <GpService/GpServiceMainTask.hpp>
#include <GpService/GpService.hpp>

namespace GPlatform {

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
