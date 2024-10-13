#pragma once

#include <GpService/ArgParser/GpServiceArgBaseDesc.hpp>
#include <GpService/Config/GpServiceCfgBaseDesc.hpp>
#include <GpCore2/GpReflection/GpReflectManager.hpp>
#include <GpCore2/GpTasks/Fibers/GpTaskFiber.hpp>

namespace GPlatform {

class GP_SERVICE_API GpServiceMainTask: public GpTaskFiber
{
public:
    CLASS_REMOVE_CTRS_MOVE_COPY(GpServiceMainTask)
    CLASS_DD(GpServiceMainTask)

protected:
                                    GpServiceMainTask   (const GpServiceArgBaseDesc& aServiceArgsDesc,
                                                         const GpServiceCfgBaseDesc& aServiceCfgDesc) noexcept;
    virtual                         ~GpServiceMainTask  (void) noexcept override;

    const GpServiceArgBaseDesc&     ServiceArgsDesc     (void) const noexcept {return iServiceArgsDesc;}

    template<typename T>
    const T&                        ServiceArgsDescAs   (void) const;

    const GpServiceCfgBaseDesc&     ServiceCfgDesc      (void) const noexcept {return iServiceCfgDesc;}

    template<typename T>
    const T&                        ServiceCfgDescAs    (void) const;

    virtual void                    OnStart             (void) override;
    virtual GpTaskRunRes::EnumT     OnStep              (void) override = 0;
    virtual void                    OnStop              (StopExceptionsT& aStopExceptionsOut) noexcept override;
    virtual void                    OnStopException     (const GpException& aException) noexcept override = 0;

private:
    const GpServiceArgBaseDesc&     iServiceArgsDesc;
    const GpServiceCfgBaseDesc&     iServiceCfgDesc;
};

template<typename T>
const T&    GpServiceMainTask::ServiceArgsDescAs (void) const
{
    return GpReflectManager::SCastRef<const T>(ServiceArgsDesc());
}

template<typename T>
const T&    GpServiceMainTask::ServiceCfgDescAs (void) const
{
    const T& serviceCfgDesc = GpReflectManager::SCastRef<const T>(ServiceCfgDesc());
    return serviceCfgDesc;
}

}// namespace GPlatform
