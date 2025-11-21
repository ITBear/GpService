#pragma once

#include <GpService/GpServiceMainTaskFactory.hpp>
#include <GpService/GpServiceMainTaskFactory.hpp>
#include <GpService/Config/GpServiceCfgBaseDescFactory.hpp>
#include <GpService/ArgParser/GpServiceArgBaseDescFactory.hpp>
#include <GpCore2/GpUtils/Other/ArgParser/GpArgParserRes.hpp>
#include <GpCore2/GpUtils/SyncPrimitives/GpConditionVarFlag.hpp>
#include <GpLog/GpLogCore/Consumers/GpLogConsumersFactory.hpp>
#include <GpService/GpServiceDelegate.hpp>
#include <GPlatform/GpService/GpLockTraceThread.hpp>

#include <csignal>
#include <future>

#if defined(GP_OS_WINDOWS)
#   include <GpCore2/Config/IncludeExt/windows.hpp>
#endif// #if defined(GP_OS_WINDOWS)

namespace GPlatform {

class GP_SERVICE_API GpService
{
public:
    CLASS_REMOVE_CTRS_DEFAULT_MOVE_COPY(GpService)
    CLASS_DD(GpService)

    enum class ForkResT
    {
        CHILD,
        PARENT
    };

    using ServiceArgOptCRefT    = GpServiceArgBaseDesc::C::Opts::CRef;
    using ResFutureT            = std::future<int>;

public:
                                GpService               (std::string aName);
                                ~GpService              (void) noexcept;

    [[nodiscard]] static int    SStartAndWaitForStop    (std::string                        aName,
                                                         size_t                             aArgc,
                                                         const char* const                  aArgv[],
                                                         GpLogConsumersFactory::UP          aLogConsumersFactoryUP,
                                                         GpLogLevel::EnumT                  aDefaultLogLevel,
                                                         GpServiceCfgBaseDescFactory::UP    aCfgBaseDescFactoryUP,
                                                         GpServiceArgBaseDescFactory::UP    aArgBaseDescFactoryUP,
                                                         GpServiceMainTaskFactory::UP       aServiceMainTaskFactoryUP,
                                                         GpServiceDelegate::UP              aServiceDelegateUP);
    [[nodiscard]] static int    SStartAndWaitForStop    (std::string                        aName,
                                                         GpLogLevel::EnumT                  aDefaultLogLevel,
                                                         GpServiceMainTaskFactory::UP       aServiceMainTaskFactoryUP);
    [[nodiscard]] static auto   SStartInSeparateThread  (std::string                        aName,
                                                         size_t                             aArgc,
                                                         const char* const                  aArgv[],
                                                         GpLogConsumersFactory::UP          aLogConsumersFactoryUP,
                                                         GpLogLevel::EnumT                  aDefaultLogLevel,
                                                         GpServiceCfgBaseDescFactory::UP    aCfgBaseDescFactoryUP,
                                                         GpServiceArgBaseDescFactory::UP    aArgBaseDescFactoryUP,
                                                         GpServiceMainTaskFactory::UP       aServiceMainTaskFactoryUP,
                                                         GpServiceDelegate::UP              aServiceDelegateUP) -> ResFutureT;

    static void                 SRequestStop            (void) noexcept;

    static GpServiceArgBaseDesc::CSP    SArgs           (void);
    static GpArgParserRes::CSP          SArgsParseRes   (void);
    static GpServiceCfgBaseDesc::CSP    SCfg            (void);
    static void                 SSetResultCode          (const int aCode);
    static std::string_view     SApplicationName        (void) noexcept;

private:
    static void                 SSystemSignalsHandler   (int aSignalId) noexcept;
#if defined(GP_OS_WINDOWS)
    static BOOL WINAPI          SWinConsoleHandler      (DWORD aSignalId) noexcept;
#endif// #if defined(GP_OS_WINDOWS)

    int                         Start                   (size_t                             aArgc,
                                                         const char* const                  aArgv[],
                                                         GpLogConsumersFactory::UP          aLogConsumersFactoryUP,
                                                         GpServiceCfgBaseDescFactory::UP    aCfgBaseDescFactoryUP,
                                                         GpServiceArgBaseDescFactory::UP    aArgBaseDescFactoryUP,
                                                         GpServiceMainTaskFactory::UP       aServiceMainTaskFactoryUP) noexcept;
    int                         Stop                    (void) noexcept;
    void                        ParseServiceArgs        (size_t                             aArgc,
                                                         const char* const                  aArgv[],
                                                         GpServiceArgBaseDescFactory::UP    aArgBaseDescFactoryUP);
    void                        ReadConfig              (GpServiceCfgBaseDescFactory::UP    aCfgBaseDescFactoryUP);
    void                        SetSystemSignalsHandler (void);
    void                        StartLog                (GpLogConsumersFactory::UP          aConsumersFactoryUP);
    void                        StartFibers             (void);
    void                        StopFibers              (void);
    void                        StartTaskScheduler      (void);
    void                        StopTaskScheduler       (void);
    void                        StartMainTask           (GpServiceMainTaskFactory::UP       aServiceMainTaskFactoryUP);
    std::string_view            Name                    (void) const noexcept;

private:
    const std::string           iName;
    GpServiceArgBaseDesc::SP    iArgsDescSP;
    GpArgParserRes::SP          iArgsParseRes;
    GpServiceCfgBaseDesc::SP    iCfgDescSP;

    GpServiceMainTask::SP       iMainTaskSP;
    GpTask::StartFutureT::SP    iMainTaskStartFutureSP;
    GpTask::DoneFutureT::SP     iMainTaskDoneFutureSP;

#if defined(GP_LOCK_TRACE)
    GpLockTraceThread           iLockTraceThread;
#endif// #if defined(GP_LOCK_TRACE)

    static GpService::UP                sServiceUP;
    static GpMutex<>                    sServiceMutex;
    static volatile std::sig_atomic_t   sSignalReceived;
    static GpConditionVarFlag           sStopFlagCV;
    static std::atomic<int>             sResultCode;
};

}// namespace GPlatform
