#pragma once

#include <GpService/GpServiceMainTaskFactory.hpp>
#include <GpService/GpServiceMainTaskFactory.hpp>
#include <GpService/Config/GpServiceCfgBaseDescFactory.hpp>
#include <GpService/ArgParser/GpServiceArgBaseDescFactory.hpp>
#include <GpCore2/GpUtils/Other/ArgParser/GpArgParserRes.hpp>
#include <GpCore2/GpUtils/SyncPrimitives/GpConditionVarFlag.hpp>
#include <GpLog/GpLogCore/Consumers/GpLogConsumersFactory.hpp>

#include <csignal>

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

    using ServiceArgOptCRefT = GpServiceArgBaseDesc::C::Opts::CRef;

public:
                                GpService                   (std::string aName);
                                ~GpService                  (void) noexcept;

    [[nodiscard]] static int    SStartAndWaitForStop        (std::string                        aName,
                                                             size_t                             aArgc,
                                                             const char* const                  aArgv[],
                                                             GpLogConsumersFactory::SP          aLogConsumersFactory,
                                                             GpLogLevel::EnumT                  aDefaultLogLevel,
                                                             GpServiceCfgBaseDescFactory::SP    aCfgBaseDescFactory,
                                                             GpServiceArgBaseDescFactory::SP    aArgBaseDescFactory,
                                                             GpServiceMainTaskFactory::SP       aServiceMainTaskFactory);
    [[nodiscard]] static int    SStartAndWaitForStop        (std::string                        aName,
                                                             GpServiceMainTaskFactory::SP       aServiceMainTaskFactory);

    static void                 SRequestStop                (void) noexcept;

    static ServiceArgOptCRefT   SArgs                       (void);
    static GpArgParserRes::CSP  SArgsParseRes               (void);
    static void                 SSetResultCode              (const int aCode);
    static std::string_view     SApplicationName            (void) noexcept;

protected:
    static void                 SInterruptWaitForStop       (void) noexcept;
    static void                 SSystemSignalsHandler       (int aSignalId) noexcept;
#if defined(GP_OS_WINDOWS)
    static BOOL WINAPI          SWinConsoleHandler          (DWORD aSignalId) noexcept;
#endif// #if defined(GP_OS_WINDOWS)


private:
    int                         Start                       (size_t                             aArgc,
                                                             const char* const                  aArgv[],
                                                             GpLogConsumersFactory::SP          aLogConsumersFactory,
                                                             GpServiceCfgBaseDescFactory::SP    aCfgBaseDescFactory,
                                                             GpServiceArgBaseDescFactory::SP    aArgBaseDescFactory,
                                                             GpServiceMainTaskFactory::SP       aServiceMainTaskFactory) noexcept;
    int                         Stop                        (void) noexcept;
    void                        ParseServiceArgs            (size_t                             aArgc,
                                                             const char* const                  aArgv[],
                                                             GpServiceArgBaseDescFactory::SP    aArgBaseDescFactory);
    void                        ReadConfig                  (GpServiceCfgBaseDescFactory::SP    aCfgBaseDescFactory);
    void                        SetSystemSignalsHandler     (void);
    void                        StartLog                    (GpLogConsumersFactory::SP          aConsumersFactory);
    void                        StartFibers                 (void);
    void                        StopFibers                  (void);
    void                        StartTaskScheduler          (void);
    void                        StopTaskScheduler           (void);
    void                        StartMainTask               (GpServiceMainTaskFactory::SP       aServiceMainTaskFactory);
    std::string_view            Name                        (void) const noexcept;

private:
    const std::string           iName;
    GpServiceArgBaseDesc::SP    iArgsDesc;
    GpArgParserRes::SP          iArgsParseRes;
    GpServiceCfgBaseDesc::SP    iCfgDesc;

    GpServiceMainTask::SP       iMainTaskSP;
    GpTask::StartFutureT::SP    iMainTaskStartFuture;
    GpTask::DoneFutureT::SP     iMainTaskDoneFuture;

    static GpService::SP                sServiceSP;
    static std::mutex                   sServiceMutex;
    static GpConditionVarFlag           sServiceCondVar;
    static std::atomic_flag             sIsStopRequested;
    static volatile std::sig_atomic_t   sSignalReceived;
    static std::atomic<int>             sResultCode;
};

}// namespace GPlatform
