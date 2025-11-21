#include <GpService/GpServiceMainTask.hpp>
#include <GpService/GpService.hpp>

namespace GPlatform {

GpServiceMainTask::GpServiceMainTask (void) noexcept
{
}

GpServiceMainTask::~GpServiceMainTask (void) noexcept
{
}

void    GpServiceMainTask::OnStart (void)
{
    // NOP
}

void    GpServiceMainTask::OnStop ([[maybe_unused]] ExceptionsT& aStopExceptionsOut) noexcept
{
    GpService::SRequestStop();
}

}// namespace GPlatform
