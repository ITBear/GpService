#include "GpServiceMainTask.hpp"
#include "GpService.hpp"

namespace GPlatform {

GpServiceMainTask::~GpServiceMainTask (void) noexcept
{
    CompleteStartPromise(MakeSP<GpItcResult>());
    GpService::SRequestStop();
}

}//namespace GPlatform
