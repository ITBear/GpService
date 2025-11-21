#include <GpService/GpServiceLib.hpp>
#include <GpCore2/GpUtils/Other/GpLinkedLibsInfo.hpp>

GP_STATIC_INITIALIZER_IMPL(GpService)
GP_LIB_REGISTRATOR(GpServiceLib)

void    GpService_StaticInitializer::OnInitialize (void)
{
    GpServiceLib::SRegisterSelf();
}
