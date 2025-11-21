#pragma once

#include <GpCore2/GpUtils/SyncPrimitives/GpLockTrace.hpp>
#include <GpCore2/GpUtils/Threads/GpThread.hpp>

#if defined(GP_USE_SYNC_PRIMITIVES)
#if defined(GP_LOCK_TRACE)

namespace GPlatform {

class GpLockTraceThread
{
public:
                GpLockTraceThread   (void) noexcept;
                ~GpLockTraceThread  (void) noexcept;

    void        Start               (void);
    void        RequestStop         (void) noexcept;
    void        Join                (void) noexcept;

private:
    GpConditionVarFlag  iStopFlag;
    GpThread::UP        iThreadUP;
};

}// namespace GPlatform

#endif// #if defined(GP_LOCK_TRACE)
#endif// #if defined(GP_USE_SYNC_PRIMITIVES)
