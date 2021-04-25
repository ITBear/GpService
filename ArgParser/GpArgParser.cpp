#include "GpArgParser.hpp"

GP_WARNING_PUSH()
GP_WARNING_DISABLE(conversion)
GP_WARNING_DISABLE(shadow)

#include <boost/program_options.hpp>
#include <boost/container/string.hpp>

GP_WARNING_POP()

namespace GPlatform {

void    GpArgParser::SParse
(
    const size_t        aArgc,
    char**              aArgv,
    GpArgBaseDesc&      aOut,
    std::string_view    aDescText
)
{
    std::string descText(aDescText);
    OptDescT optDesc(descText);

    SFillOptions(optDesc, aOut);
    SParseOptions(aArgc, aArgv, optDesc, aOut);
}

void    GpArgParser::SFillOptions
(
    OptDescT&               aOptDesc,
    const GpArgBaseDesc&    aOut
)
{
    aOptDesc.add_options()("help", "Show help message");

    const GpTypeStructInfo& typeInfo = aOut.TypeInfo();

    for (const GpTypePropInfo& propInfo: typeInfo.Props())
    {
        const std::string               propName        = std::string(propInfo.Name());
        const GpType::EnumT             propType        = propInfo.Type();
        const GpTypeContainer::EnumT    propContainer   = propInfo.Container();

        THROW_GPE_COND
        (
               (propContainer == GpTypeContainer::NO)
            || (propContainer == GpTypeContainer::VECTOR),
            [&](){return "Property '"_sv + propName + "' container must be NO or VECTOR"_sv;}
        );

        switch (propType)
        {
            case GpType::S_INT_8:    [[fallthrough]];
            case GpType::U_INT_8:    [[fallthrough]];
            case GpType::S_INT_16:   [[fallthrough]];
            case GpType::U_INT_16:   [[fallthrough]];
            case GpType::S_INT_32:   [[fallthrough]];
            case GpType::U_INT_32:   [[fallthrough]];
            case GpType::S_INT_64:   [[fallthrough]];
            case GpType::U_INT_64:   [[fallthrough]];
            case GpType::UNIX_TS_S:  [[fallthrough]];
            case GpType::UNIX_TS_MS: [[fallthrough]];
            case GpType::DOUBLE:     [[fallthrough]];
            case GpType::FLOAT:      [[fallthrough]];
            case GpType::UUID:       [[fallthrough]];
            case GpType::STRING:     [[fallthrough]];
            case GpType::BLOB:       [[fallthrough]];
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
            case GpType::ENUM_FLAGS:
            {
                if (propContainer == GpTypeContainer::NO)
                {
                    aOptDesc.add_options()(propName.data(), boost::program_options::value<std::vector<boost::container::string>>()->multitoken(), "");
                } else
                {
                    THROW_GPE("Unsupported container type '"_sv + GpTypeContainer::SToString(propContainer) + "', property name '"_sv + propName + "'"_sv);
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
            case GpType::STRUCT:    [[fallthrough]];
            case GpType::STRUCT_SP: [[fallthrough]];
            case GpType::NOT_SET:   [[fallthrough]];
            default:
            {
                THROW_GPE("Unsupported type '"_sv + GpType::SToString(propType) + "', property name '"_sv + propName + "'"_sv);
            }
        }
    }
}

void    GpArgParser::SParseOptions
(
    const size_t        aArgc,
    char**              aArgv,
    const OptDescT&     aOptDesc,
    GpArgBaseDesc&      aOut
)
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
            THROW_GPE_COND
            (
                !optVal.empty(),
                [&](){return "Empty value for property '"_sv + propName + "'"_sv;}
            );

            if (propContainer == GpTypeContainer::NO)
            {
                switch (propType)
                {
                    case GpType::S_INT_8:
                    {
                        const auto&     v   = optVal.as<boost::container::string>();
                        GpRawPtrCharR   sv  = {v.data(), v.size()};
                        propInfo.Value_SInt8(aOut)  = NumOps::SConvert<s_int_8>(StrOps::SToSI64(sv));
                    } break;
                    case GpType::U_INT_8:
                    {
                        const auto&     v   = optVal.as<boost::container::string>();
                        GpRawPtrCharR   sv  = {v.data(), v.size()};
                        propInfo.Value_UInt8(aOut)  = NumOps::SConvert<u_int_8>(StrOps::SToUI64(sv));
                    } break;
                    case GpType::S_INT_16:
                    {
                        const auto&     v   = optVal.as<boost::container::string>();
                        GpRawPtrCharR   sv  = {v.data(), v.size()};
                        propInfo.Value_SInt16(aOut) = NumOps::SConvert<s_int_16>(StrOps::SToSI64(sv));
                    } break;
                    case GpType::U_INT_16:
                    {
                        const auto&     v   = optVal.as<boost::container::string>();
                        GpRawPtrCharR   sv  = {v.data(), v.size()};
                        propInfo.Value_UInt16(aOut) = NumOps::SConvert<u_int_16>(StrOps::SToUI64(sv));
                    } break;
                    case GpType::S_INT_32:
                    {
                        const auto&     v   = optVal.as<boost::container::string>();
                        GpRawPtrCharR   sv  = {v.data(), v.size()};
                        propInfo.Value_SInt32(aOut) = NumOps::SConvert<s_int_32>(StrOps::SToSI64(sv));
                    } break;
                    case GpType::U_INT_32:
                    {
                        const auto&     v   = optVal.as<boost::container::string>();
                        GpRawPtrCharR   sv  = {v.data(), v.size()};
                        propInfo.Value_UInt32(aOut) = NumOps::SConvert<u_int_32>(StrOps::SToUI64(sv));
                    } break;
                    case GpType::S_INT_64:  [[fallthrough]];
                    case GpType::UNIX_TS_S: [[fallthrough]];
                    case GpType::UNIX_TS_MS:
                    {
                        const auto&     v   = optVal.as<boost::container::string>();
                        GpRawPtrCharR   sv  = {v.data(), v.size()};
                        propInfo.Value_SInt64(aOut) = NumOps::SConvert<s_int_64>(StrOps::SToSI64(sv));
                    } break;
                    case GpType::U_INT_64:
                    {
                        const auto&     v   = optVal.as<boost::container::string>();
                        GpRawPtrCharR   sv  = {v.data(), v.size()};
                        propInfo.Value_UInt64(aOut) = NumOps::SConvert<u_int_64>(StrOps::SToUI64(sv));
                    } break;
                    case GpType::DOUBLE:
                    {
                        const auto&     v   = optVal.as<boost::container::string>();
                        GpRawPtrCharR   sv  = {v.data(), v.size()};
                        propInfo.Value_Double(aOut) = NumOps::SConvert<double>(StrOps::SToDouble_fast(sv));
                    } break;
                    case GpType::FLOAT:
                    {
                        const auto&     v   = optVal.as<boost::container::string>();
                        GpRawPtrCharR   sv  = {v.data(), v.size()};
                        propInfo.Value_Float(aOut)  = NumOps::SConvert<float>(StrOps::SToDouble_fast(sv));
                    } break;
                    case GpType::UUID:
                    {
                        const auto&     v   = optVal.as<boost::container::string>();
                        GpRawPtrCharR   sv  = {v.data(), v.size()};
                        propInfo.Value_UUID(aOut)   = GpUUID::SFromString(sv);
                    } break;
                    case GpType::STRING:
                    {
                        const auto&     v   = optVal.as<boost::container::string>();
                        GpRawPtrCharR   sv  = {v.data(), v.size()};
                        propInfo.Value_String(aOut) = sv.AsStringView();
                    } break;
                    case GpType::BLOB:
                    {
                        const auto&     v   = optVal.as<boost::container::string>();
                        GpRawPtrCharR   sv  = {v.data(), v.size()};
                        propInfo.Value_BLOB(aOut)   = StrOps::SToBytesHex(sv);
                    } break;
                    case GpType::ENUM:
                    {
                        const auto&     v   = optVal.as<boost::container::string>();
                        GpRawPtrCharR   sv  = {v.data(), v.size()};
                        propInfo.Value_Enum(aOut).FromString(sv);
                    } break;
                    case GpType::ENUM_FLAGS:
                    {
                        const auto& v = optVal.as<std::vector<boost::container::string>>();
                        GpVector<std::string> vs;
                        for (const auto& s: v)
                        {
                            vs.emplace_back(s);
                        }

                        propInfo.Value_EnumFlags(aOut).FromStringArray(vs);
                    } break;
                    case GpType::BOOLEAN:[[fallthrough]];
                    case GpType::STRUCT:[[fallthrough]];
                    case GpType::STRUCT_SP:[[fallthrough]];
                    case GpType::NOT_SET:[[fallthrough]];
                    default:
                    {
                        THROW_GPE("Unsupported type '"_sv + GpType::SToString(propType) + "' of prop '"_sv + propName + "'"_sv);
                    }
                }//switch (propType)
            } else
            {
                const auto& v = optVal.as<std::vector<boost::container::string>>();

                switch (propType)
                {
                    case GpType::S_INT_8:
                    {
                        auto& vec = propInfo.Value_Vec_SInt8(aOut); vec.clear();
                        for (const auto& e: v) vec.emplace_back(NumOps::SConvert<s_int_8>(StrOps::SToSI64({e.data(), e.size()})));
                    } break;
                    case GpType::U_INT_8:
                    {
                        auto& vec = propInfo.Value_Vec_UInt8(aOut); vec.clear();
                        for (const auto& e: v) vec.emplace_back(NumOps::SConvert<u_int_8>(StrOps::SToUI64({e.data(), e.size()})));
                    } break;
                    case GpType::S_INT_16:
                    {
                        auto& vec = propInfo.Value_Vec_SInt16(aOut); vec.clear();
                        for (const auto& e: v) vec.emplace_back(NumOps::SConvert<s_int_16>(StrOps::SToSI64({e.data(), e.size()})));
                    } break;
                    case GpType::U_INT_16:
                    {
                        auto& vec = propInfo.Value_Vec_UInt16(aOut); vec.clear();
                        for (const auto& e: v) vec.emplace_back(NumOps::SConvert<u_int_16>(StrOps::SToUI64({e.data(), e.size()})));
                    } break;
                    case GpType::S_INT_32:
                    {
                        auto& vec = propInfo.Value_Vec_SInt32(aOut); vec.clear();
                        for (const auto& e: v) vec.emplace_back(NumOps::SConvert<s_int_32>(StrOps::SToSI64({e.data(), e.size()})));
                    } break;
                    case GpType::U_INT_32:
                    {
                        auto& vec = propInfo.Value_Vec_UInt32(aOut); vec.clear();
                        for (const auto& e: v) vec.emplace_back(NumOps::SConvert<u_int_32>(StrOps::SToUI64({e.data(), e.size()})));
                    } break;
                    case GpType::S_INT_64:  [[fallthrough]];
                    case GpType::UNIX_TS_S: [[fallthrough]];
                    case GpType::UNIX_TS_MS:
                    {
                        auto& vec = propInfo.Value_Vec_SInt64(aOut); vec.clear();
                        for (const auto& e: v) vec.emplace_back(NumOps::SConvert<s_int_64>(StrOps::SToSI64({e.data(), e.size()})));
                    } break;
                    case GpType::U_INT_64:
                    {
                        auto& vec = propInfo.Value_Vec_UInt64(aOut); vec.clear();
                        for (const auto& e: v) vec.emplace_back(NumOps::SConvert<u_int_64>(StrOps::SToUI64({e.data(), e.size()})));
                    } break;
                    case GpType::DOUBLE:
                    {
                        auto& vec = propInfo.Value_Vec_Double(aOut); vec.clear();
                        for (const auto& e: v) vec.emplace_back(NumOps::SConvert<double>(StrOps::SToDouble_fast({e.data(), e.size()})));
                    } break;
                    case GpType::FLOAT:
                    {
                        auto& vec = propInfo.Value_Vec_Float(aOut); vec.clear();
                        for (const auto& e: v) vec.emplace_back(NumOps::SConvert<float>(StrOps::SToDouble_fast({e.data(), e.size()})));
                    } break;
                    case GpType::UUID:
                    {
                        auto& vec = propInfo.Value_Vec_UUID(aOut); vec.clear();
                        for (const auto& e: v) vec.emplace_back(GpUUID::SFromString({e.data(), e.size()}));
                    } break;
                    case GpType::STRING:
                    {
                        auto& vec = propInfo.Value_Vec_String(aOut); vec.clear();
                        for (const auto& e: v)
                        {
                            GpRawPtrCharR   sv  = {e.data(), e.size()};
                            vec.emplace_back(sv.AsStringView());
                        }
                    } break;
                    case GpType::BLOB:
                    {
                        auto& vec = propInfo.Value_Vec_BLOB(aOut); vec.clear();
                        for (const auto& e: v) vec.emplace_back(StrOps::SToBytesHex({e.data(), e.size()}));
                    } break;
                    case GpType::ENUM:[[fallthrough]];
                    case GpType::ENUM_FLAGS:[[fallthrough]];
                    case GpType::BOOLEAN:[[fallthrough]];
                    case GpType::STRUCT:[[fallthrough]];
                    case GpType::STRUCT_SP:[[fallthrough]];
                    case GpType::NOT_SET:[[fallthrough]];
                    default:
                    {
                        THROW_GPE("Unsupported array of type '"_sv + GpType::SToString(propType) + "' of prop '"_sv + propName + "'"_sv);
                    }
                }
            }
        }
    }
}

}//namespace GPlatform
