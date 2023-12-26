#pragma once

#include "GpServiceMainTaskFactory.hpp"
#include "Config/GpServiceCfgBaseDescFactory.hpp"
#include "ArgParser/GpServiceArgBaseDescFactory.hpp"
#include <GpCore2/GpUtils/SyncPrimitives/GpConditionVarFlag.hpp>
#include <GpCore2/GpUtils/Other/GpLinkedLibsInfo.hpp>
#include <GpLog/GpLogCore/Consumers/GpLogConsumersFactory.hpp>

#if defined(GP_POSIX)
#   include <csignal>
#endif// #if defined(GP_POSIX)

namespace GPlatform {

GP_DECLARE_LIB(GP_SERVICE_API, GpServiceLib)

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

public:
    inline                                      GpService               (std::u8string aName);
                                                ~GpService              (void) noexcept;

    [[nodiscard]] static int                    SStartAndWaitForStop    (std::u8string                      aName,
                                                                         const size_t                       aArgc,
                                                                         char**                             aArgv,
                                                                         GpLogConsumersFactory::SP          aLogConsumersFactory,
                                                                         GpServiceCfgBaseDescFactory::SP    aCfgBaseDescFactory,
                                                                         GpServiceArgBaseDescFactory::SP    aArgBaseDescFactory,
                                                                         GpServiceMainTaskFactory::SP       aServiceMainTaskFactory);
    [[nodiscard]] static int                    SStartAndWaitForStop    (std::u8string                      aName,
                                                                         GpServiceMainTaskFactory::SP       aServiceMainTaskFactory);

    static void                                 SRequestStop            (void) noexcept;
    static GpServiceArgBaseDesc::C::Opt::CRef   SArgs                   (void);

protected:
    static void                                 SInterruptWaitForStop   (void) noexcept;

    static void                                 SSystemSignalsHandler   (int aSignalId) noexcept;
    static void                                 SPrintStacktrace        (void);

private:
    int                                         Start                   (const size_t                       aArgc,
                                                                         char**                             aArgv,
                                                                         GpLogConsumersFactory::SP          aLogConsumersFactory,
                                                                         GpServiceCfgBaseDescFactory::SP    aCfgBaseDescFactory,
                                                                         GpServiceArgBaseDescFactory::SP    aArgBaseDescFactory,
                                                                         GpServiceMainTaskFactory::SP       aServiceMainTaskFactory) noexcept;
    int                                         Stop                    (void) noexcept;
    void                                        ParseServiceArgs        (const size_t                       aArgc,
                                                                         char**                             aArgv,
                                                                         GpServiceArgBaseDescFactory::SP    aArgBaseDescFactory);
    void                                        ReadConfig              (GpServiceCfgBaseDescFactory::SP    aCfgBaseDescFactory);
    void                                        SetSystemSignalsHandler (void);
    void                                        StartLog                (GpLogConsumersFactory::SP          aConsumersFactory);
    void                                        StartFibers             (void);
    void                                        StopFibers              (void);
    void                                        StartTaskScheduler      (void);
    void                                        StopTaskScheduler       (void);
    void                                        StartMainTask           (GpServiceMainTaskFactory::SP       aServiceMainTaskFactory);
    void                                        CheckMainTaskDoneResult (void);

private:
    const std::u8string                         iName;
    GpServiceArgBaseDesc::SP                    iArgsDesc;
    GpServiceCfgBaseDesc::SP                    iCfgDesc;

    GpServiceMainTask::SP                       iMainTaskSP;
    GpTask::StartFutureT::SP                    iMainTaskStartFuture;
    GpTask::DoneFutureT::SP                     iMainTaskDoneFuture;

    static GpService::SP                        sService;
    static std::mutex                           sServiceMutex;
    static GpConditionVarFlag                   sServiceCondVar;
    static std::atomic_flag                     sIsStopRequested;

#if defined(GP_POSIX)
    static volatile std::sig_atomic_t           sSignalReceived;
#endif// #if defined(GP_POSIX)
};

GpService::GpService (std::u8string aName):
iName(std::move(aName))
{
}

}//GPlatform
