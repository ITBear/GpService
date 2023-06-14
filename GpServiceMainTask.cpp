#include "GpServiceMainTask.hpp"
#include "GpService.hpp"

namespace GPlatform {

GpServiceMainTask::~GpServiceMainTask (void) noexcept
{
    CompleteStartPromise(MakeSP<StartItcResultT>(size_t(0)));
    GpService::SRequestStop();
}

}//namespace GPlatform
