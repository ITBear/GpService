#pragma once

#include <GpCore2/GpUtils/Macro/GpMacroClass.hpp>
#include <GpCore2/GpUtils/Types/Containers/GpContainersT.hpp>

namespace GPlatform {

class GpServiceDelegate 
{
public:
    CLASS_REMOVE_CTRS_MOVE_COPY(GpServiceDelegate)
    CLASS_DD(GpServiceDelegate)
    
public:
                    GpServiceDelegate   (void) noexcept = default;
    virtual         ~GpServiceDelegate  (void) noexcept = default;
    
    virtual void    OnStart             (void) = 0;
    virtual void    OnStep              (void) = 0;
    virtual void    OnStop              (void) noexcept = 0;
    virtual void    OnError             (const std::exception& aEx) noexcept = 0;
};

}// namespace GPlatform
