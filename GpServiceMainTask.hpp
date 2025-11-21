#pragma once

#include <GpService/ArgParser/GpServiceArgBaseDesc.hpp>
#include <GpCore2/GpTasks/Fibers/GpTaskFiber.hpp>

namespace GPlatform {

class GP_SERVICE_API GpServiceMainTask: public GpTaskFiber
{
public:
    CLASS_REMOVE_CTRS_MOVE_COPY(GpServiceMainTask)
    CLASS_DD(GpServiceMainTask)

protected:
                                    GpServiceMainTask   (void) noexcept;

public:
    virtual                         ~GpServiceMainTask  (void) noexcept override;

protected:
    virtual void                    OnStart             (void) override;
    virtual GpTaskRunRes::EnumT     OnStep              (void) override = 0;
    virtual void                    OnStop              (ExceptionsT& aStopExceptionsOut) noexcept override;
    virtual void                    OnStopException     (const GpException& aException) noexcept override = 0;
};

}// namespace GPlatform
