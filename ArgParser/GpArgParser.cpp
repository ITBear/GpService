#include "GpArgParser.hpp"

GP_WARNING_PUSH()
GP_WARNING_DISABLE(conversion)

#include <boost/program_options.hpp>
#include <boost/container/string.hpp>

GP_WARNING_POP()

namespace GPlatform {

void    GpArgParser::SParse (const size_t       aArgc,
                             char**             aArgv,
                             GpArgBaseDesc&     aOut,
                             std::string_view   aDescText)
{
    std::string descText(aDescText);
    OptDescT optDesc(descText);

    SFillOptions(optDesc, aOut);
    SParseOptions(aArgc, aArgv, optDesc, aOut);
}

void    GpArgParser::SFillOptions (OptDescT&            aOptDesc,
                                   const GpArgBaseDesc& aOut)
{
    aOptDesc.add_options()("help", "Show help message");

    const GpTypeStructInfo& typeInfo = aOut.TypeInfo();

    for (const GpTypePropInfo& propInfo: typeInfo.Props())
    {
        const std::string               propName        = std::string(propInfo.Name());
        const GpType::EnumT             propType        = propInfo.Type();
        const GpTypeContainer::EnumT    propContainer   = propInfo.Container();

        THROW_GPE_COND_CHECK_M(   (propContainer == GpTypeContainer::NO)
                               || (propContainer == GpTypeContainer::VECTOR),
                               "Property '"_sv + propName + "' container must be NO or VECTOR"_sv);

        switch (propType)
        {
            case GpType::S_INT_8:
            case GpType::U_INT_8:
            case GpType::S_INT_16:
            case GpType::U_INT_16:
            case GpType::S_INT_32:
            case GpType::U_INT_32:
            case GpType::S_INT_64:
            case GpType::U_INT_64:
            case GpType::DOUBLE:
            case GpType::FLOAT:
            case GpType::UUID:
            case GpType::STRING:
            case GpType::BLOB:
            case GpType::ENUM:
            {
                if (propContainer == GpTypeContainer::NO)
                {
                    aOptDesc.add_options()(propName.data(), boost::program_options::value<boost::container::string>(), "");
                } else
                {
                    aOptDesc.add_options()(propName.data(), boost::program_options::value<std::vector<boost::container::string>>()->multitoken(), "");
                }
            } break;
            case GpType::BOOLEAN:
            {
                if (propContainer == GpTypeContainer::NO)
                {
                    aOptDesc.add_options()(propName.data(), "");
                } else
                {
                    THROW_GPE("Unsupported container type '"_sv + GpTypeContainer::SToString(propContainer) + "', property name '"_sv + propName + "'"_sv);
                }
            } break;
            case GpType::STRUCT:
            case GpType::STRUCT_SP:
            case GpType::NOT_SET:
            default:
            {
                THROW_GPE("Unsupported type '"_sv + GpType::SToString(propType) + "', property name '"_sv + propName + "'"_sv);
            }
        }
    }
}

void    GpArgParser::SParseOptions (const size_t        aArgc,
                                    char**              aArgv,
                                    const OptDescT&     aOptDesc,
                                    GpArgBaseDesc&      aOut)
{
    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line(NumOps::SConvert<int>(aArgc), aArgv, aOptDesc), vm);
    boost::program_options::notify(vm);

    const GpTypeStructInfo& typeInfo = aOut.TypeInfo();

    for (const GpTypePropInfo& propInfo: typeInfo.Props())
    {
        const std::string               propName        = std::string(propInfo.Name());
        const GpType::EnumT             propType        = propInfo.Type();
        const GpTypeContainer::EnumT    propContainer   = propInfo.Container();

        if (!vm.count(propName.data()))
        {
            continue;
        }

        const auto& optVal = vm[propName.data()];

        if (propType == GpType::BOOLEAN)
        {
            propInfo.Value_Bool(aOut) = true;
        } else
        {
            THROW_GPE_COND_CHECK_M(!optVal.empty(), "Empty value for property '"_sv + propName + "'"_sv);

            if (propContainer == GpTypeContainer::NO)
            {
                const auto&     v   = optVal.as<boost::container::string>();
                GpRawPtrCharR   sv  = {v.data(), v.size()};

                     if (propType == GpType::S_INT_8)   propInfo.Value_SInt8(aOut)  = NumOps::SConvert<s_int_8>(GpStringOps::SToSI64(sv));
                else if (propType == GpType::U_INT_8)   propInfo.Value_UInt8(aOut)  = NumOps::SConvert<u_int_8>(GpStringOps::SToUI64(sv));
                else if (propType == GpType::S_INT_16)  propInfo.Value_SInt16(aOut) = NumOps::SConvert<s_int_16>(GpStringOps::SToSI64(sv));
                else if (propType == GpType::U_INT_16)  propInfo.Value_UInt16(aOut) = NumOps::SConvert<u_int_16>(GpStringOps::SToUI64(sv));
                else if (propType == GpType::S_INT_32)  propInfo.Value_SInt32(aOut) = NumOps::SConvert<s_int_32>(GpStringOps::SToSI64(sv));
                else if (propType == GpType::U_INT_32)  propInfo.Value_UInt32(aOut) = NumOps::SConvert<u_int_32>(GpStringOps::SToUI64(sv));
                else if (propType == GpType::S_INT_64)  propInfo.Value_SInt64(aOut) = NumOps::SConvert<s_int_64>(GpStringOps::SToSI64(sv));
                else if (propType == GpType::U_INT_64)  propInfo.Value_UInt64(aOut) = NumOps::SConvert<u_int_64>(GpStringOps::SToUI64(sv));
                else if (propType == GpType::DOUBLE)    propInfo.Value_Double(aOut) = NumOps::SConvert<double>(GpStringOps::SToDouble_fast(sv));
                else if (propType == GpType::FLOAT)     propInfo.Value_Float(aOut)  = NumOps::SConvert<float>(GpStringOps::SToDouble_fast(sv));
                else if (propType == GpType::UUID)      propInfo.Value_UUID(aOut)   = GpUUID::SFromString(sv);
                else if (propType == GpType::STRING)    propInfo.Value_String(aOut) = sv.AsStringView();
                else if (propType == GpType::BLOB)      propInfo.Value_BLOB(aOut)   = GpStringOps::SToBytes(sv);
                else if (propType == GpType::ENUM)      propInfo.Value_Enum(aOut).FromString(sv);
                else THROW_GPE("Unsupported type '"_sv + GpType::SToString(propType) + "' of prop '"_sv + propName + "'"_sv);
            } else
            {
                const auto& v = optVal.as<std::vector<boost::container::string>>();

                if (propType == GpType::S_INT_8)
                {
                    auto& vec = propInfo.Value_Vec_SInt8(aOut); vec.clear();
                    for (const auto& e: v) vec.emplace_back(NumOps::SConvert<s_int_8>(GpStringOps::SToSI64({e.data(), e.size()})));
                } else if (propType == GpType::U_INT_8)
                {
                    auto& vec = propInfo.Value_Vec_UInt8(aOut); vec.clear();
                    for (const auto& e: v) vec.emplace_back(NumOps::SConvert<u_int_8>(GpStringOps::SToUI64({e.data(), e.size()})));
                } else if (propType == GpType::S_INT_16)
                {
                    auto& vec = propInfo.Value_Vec_SInt16(aOut); vec.clear();
                    for (const auto& e: v) vec.emplace_back(NumOps::SConvert<s_int_16>(GpStringOps::SToSI64({e.data(), e.size()})));
                } else if (propType == GpType::U_INT_16)
                {
                    auto& vec = propInfo.Value_Vec_UInt16(aOut); vec.clear();
                    for (const auto& e: v) vec.emplace_back(NumOps::SConvert<u_int_16>(GpStringOps::SToUI64({e.data(), e.size()})));
                } else if (propType == GpType::S_INT_32)
                {
                    auto& vec = propInfo.Value_Vec_SInt32(aOut); vec.clear();
                    for (const auto& e: v) vec.emplace_back(NumOps::SConvert<s_int_32>(GpStringOps::SToSI64({e.data(), e.size()})));
                } else if (propType == GpType::U_INT_32)
                {
                    auto& vec = propInfo.Value_Vec_UInt32(aOut); vec.clear();
                    for (const auto& e: v) vec.emplace_back(NumOps::SConvert<u_int_32>(GpStringOps::SToUI64({e.data(), e.size()})));
                } else if (propType == GpType::S_INT_64)
                {
                    auto& vec = propInfo.Value_Vec_SInt64(aOut); vec.clear();
                    for (const auto& e: v) vec.emplace_back(NumOps::SConvert<s_int_64>(GpStringOps::SToSI64({e.data(), e.size()})));
                } else if (propType == GpType::U_INT_64)
                {
                    auto& vec = propInfo.Value_Vec_UInt64(aOut); vec.clear();
                    for (const auto& e: v) vec.emplace_back(NumOps::SConvert<u_int_64>(GpStringOps::SToUI64({e.data(), e.size()})));
                } else if (propType == GpType::DOUBLE)
                {
                    auto& vec = propInfo.Value_Vec_Double(aOut); vec.clear();
                    for (const auto& e: v) vec.emplace_back(NumOps::SConvert<double>(GpStringOps::SToDouble_fast({e.data(), e.size()})));
                } else if (propType == GpType::FLOAT)
                {
                    auto& vec = propInfo.Value_Vec_Float(aOut); vec.clear();
                    for (const auto& e: v) vec.emplace_back(NumOps::SConvert<float>(GpStringOps::SToDouble_fast({e.data(), e.size()})));
                } else if (propType == GpType::UUID)
                {
                    auto& vec = propInfo.Value_Vec_UUID(aOut); vec.clear();
                    for (const auto& e: v) vec.emplace_back(GpUUID::SFromString({e.data(), e.size()}));
                } else if (propType == GpType::STRING)
                {
                    auto& vec = propInfo.Value_Vec_String(aOut); vec.clear();
                    for (const auto& e: v)
                    {
                        GpRawPtrCharR   sv  = {e.data(), e.size()};
                        vec.emplace_back(sv.AsStringView());
                    }
                } else if (propType == GpType::BLOB)
                {
                    auto& vec = propInfo.Value_Vec_BLOB(aOut); vec.clear();
                    for (const auto& e: v) vec.emplace_back(GpStringOps::SToBytes({e.data(), e.size()}));
                } else
                {
                    THROW_GPE("Unsupported type '"_sv + GpType::SToString(propType) + "' of prop '"_sv + propName + "'"_sv);
                }
            }
        }
    }
}

}//namespace GPlatform
