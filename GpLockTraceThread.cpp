#include <GPlatform/GpService/GpLockTraceThread.hpp>
#include <GpCore2/GpUtils/DateTime/GpDateTimeOps.hpp>

#if defined(GP_USE_SYNC_PRIMITIVES)
#if defined(GP_LOCK_TRACE)

namespace GPlatform {

GpLockTraceThread::GpLockTraceThread (void) noexcept
{
}

GpLockTraceThread::~GpLockTraceThread (void) noexcept
{
}

void    GpLockTraceThread::Start (void)
{
    iThreadUP = std::make_unique<GpThread>
    (
        iStopFlag,
        "LockTrace"
    );

    iThreadUP->Run
    (
        [](GpConditionVarFlag& aStopFlag)
        {
            unix_ts_ms_t beginTs = GpDateTimeOps::SUnixTS_ms();

            while (aStopFlag.Test() == false)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                const auto nowTs = GpDateTimeOps::SUnixTS_ms();

                if ((nowTs - beginTs) < 2.0_unix_ts_s)
                {
                    continue;
                }

                beginTs = nowTs;

                GpLockTrace& lockTrace = GpLockTrace::S();
                lockTrace.Check(5.0_unix_ts_s);
            }
        }
    );
}

void    GpLockTraceThread::RequestStop (void) noexcept
{
    if (iThreadUP)
    {
        iThreadUP->RequestStop();
    }
}

void    GpLockTraceThread::Join (void) noexcept
{
    if (iThreadUP)
    {
        iThreadUP->Join();
        iThreadUP.reset();
    }
}

}// namespace GPlatform

#endif// #if defined(GP_LOCK_TRACE)
#endif// #if defined(GP_USE_SYNC_PRIMITIVES)
