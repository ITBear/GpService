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

    const GpReflectModel& model = aOut.ReflectModel();

    for (const GpReflectProp& propInfo: model.Props())
    {
        const std::string                   propName        = std::string(propInfo.Name());
        const GpReflectType::EnumT          propType        = propInfo.Type();
        const GpReflectContainerType::EnumT propContainer   = propInfo.Container();

        THROW_COND_GP
        (
               (propContainer == GpReflectContainerType::NO)
            || (propContainer == GpReflectContainerType::VECTOR),
            [&](){return "Property '"_sv + propName + "' container must be NO or VECTOR"_sv;}
        );

        switch (propType)
        {
            case GpReflectType::S_INT_8:    [[fallthrough]];
            case GpReflectType::U_INT_8:    [[fallthrough]];
            case GpReflectType::S_INT_16:   [[fallthrough]];
            case GpReflectType::U_INT_16:   [[fallthrough]];
            case GpReflectType::S_INT_32:   [[fallthrough]];
            case GpReflectType::U_INT_32:   [[fallthrough]];
            case GpReflectType::S_INT_64:   [[fallthrough]];
            case GpReflectType::U_INT_64:   [[fallthrough]];
            case GpReflectType::DOUBLE:     [[fallthrough]];
            case GpReflectType::FLOAT:      [[fallthrough]];
            case GpReflectType::UUID:       [[fallthrough]];
            case GpReflectType::STRING:     [[fallthrough]];
            case GpReflectType::BLOB:       [[fallthrough]];
            case GpReflectType::ENUM:
            {
                if (propContainer == GpReflectContainerType::NO)
                {
                    aOptDesc.add_options()(propName.data(), boost::program_options::value<boost::container::string>(), "");
                } else
                {
                    aOptDesc.add_options()(propName.data(), boost::program_options::value<std::vector<boost::container::string>>()->multitoken(), "");
                }
            } break;
            case GpReflectType::ENUM_FLAGS:
            {
                if (propContainer == GpReflectContainerType::NO)
                {
                    aOptDesc.add_options()(propName.data(), boost::program_options::value<std::vector<boost::container::string>>()->multitoken(), "");
                } else
                {
                    THROW_GP("Unsupported container type '"_sv + GpReflectContainerType::SToString(propContainer) + "', property name '"_sv + propName + "'"_sv);
                }
            } break;
            case GpReflectType::BOOLEAN:
            {
                if (propContainer == GpReflectContainerType::NO)
                {
                    aOptDesc.add_options()(propName.data(), "");
                } else
                {
                    THROW_GP("Unsupported container type '"_sv + GpReflectContainerType::SToString(propContainer) + "', property name '"_sv + propName + "'"_sv);
                }
            } break;
            case GpReflectType::OBJECT:    [[fallthrough]];
            case GpReflectType::OBJECT_SP: [[fallthrough]];
            case GpReflectType::NOT_SET:   [[fallthrough]];
            default:
            {
                THROW_GP("Unsupported type '"_sv + GpReflectType::SToString(propType) + "', property name '"_sv + propName + "'"_sv);
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

    const GpReflectModel&   model   = aOut.ReflectModel();
    void*                   dataPtr = aOut.ReflectDataPtr();

    for (const GpReflectProp& propInfo: model.Props())
    {
        const std::string                   propName        = std::string(propInfo.Name());
        const GpReflectType::EnumT          propType        = propInfo.Type();
        const GpReflectContainerType::EnumT propContainer   = propInfo.Container();

        if (!vm.count(propName.data()))
        {
            continue;
        }

        const auto& optVal = vm[propName.data()];

        if (propType == GpReflectType::BOOLEAN)
        {
            propInfo.Value_Bool(dataPtr) = true;
        } else
        {
            THROW_COND_GP
            (
                !optVal.empty(),
                [&](){return "Empty value for property '"_sv + propName + "'"_sv;}
            );

            if (propContainer == GpReflectContainerType::NO)
            {
                switch (propType)
                {
                    case GpReflectType::S_INT_8:
                    {
                        const auto&         v   = optVal.as<boost::container::string>();
                        std::string_view    sv  = {v.data(), v.size()};
                        propInfo.Value_SInt8(dataPtr) = NumOps::SConvert<s_int_8>(StrOps::SToSI64(sv));
                    } break;
                    case GpReflectType::U_INT_8:
                    {
                        const auto&         v   = optVal.as<boost::container::string>();
                        std::string_view    sv  = {v.data(), v.size()};
                        propInfo.Value_UInt8(dataPtr) = NumOps::SConvert<u_int_8>(StrOps::SToUI64(sv));
                    } break;
                    case GpReflectType::S_INT_16:
                    {
                        const auto&         v   = optVal.as<boost::container::string>();
                        std::string_view    sv  = {v.data(), v.size()};
                        propInfo.Value_SInt16(dataPtr) = NumOps::SConvert<s_int_16>(StrOps::SToSI64(sv));
                    } break;
                    case GpReflectType::U_INT_16:
                    {
                        const auto&         v   = optVal.as<boost::container::string>();
                        std::string_view    sv  = {v.data(), v.size()};
                        propInfo.Value_UInt16(dataPtr) = NumOps::SConvert<u_int_16>(StrOps::SToUI64(sv));
                    } break;
                    case GpReflectType::S_INT_32:
                    {
                        const auto&         v   = optVal.as<boost::container::string>();
                        std::string_view    sv  = {v.data(), v.size()};
                        propInfo.Value_SInt32(dataPtr) = NumOps::SConvert<s_int_32>(StrOps::SToSI64(sv));
                    } break;
                    case GpReflectType::U_INT_32:
                    {
                        const auto&         v   = optVal.as<boost::container::string>();
                        std::string_view    sv  = {v.data(), v.size()};
                        propInfo.Value_UInt32(dataPtr) = NumOps::SConvert<u_int_32>(StrOps::SToUI64(sv));
                    } break;
                    case GpReflectType::S_INT_64:
                    {
                        const auto&         v   = optVal.as<boost::container::string>();
                        std::string_view    sv  = {v.data(), v.size()};
                        propInfo.Value_SInt64(dataPtr) = NumOps::SConvert<s_int_64>(StrOps::SToSI64(sv));
                    } break;
                    case GpReflectType::U_INT_64:
                    {
                        const auto&         v   = optVal.as<boost::container::string>();
                        std::string_view    sv  = {v.data(), v.size()};
                        propInfo.Value_UInt64(dataPtr) = NumOps::SConvert<u_int_64>(StrOps::SToUI64(sv));
                    } break;
                    case GpReflectType::DOUBLE:
                    {
                        const auto&         v   = optVal.as<boost::container::string>();
                        std::string_view    sv  = {v.data(), v.size()};
                        propInfo.Value_Double(dataPtr) = NumOps::SConvert<double>(StrOps::SToDouble(sv));
                    } break;
                    case GpReflectType::FLOAT:
                    {
                        const auto&         v   = optVal.as<boost::container::string>();
                        std::string_view    sv  = {v.data(), v.size()};
                        propInfo.Value_Float(dataPtr) = NumOps::SConvert<float>(StrOps::SToDouble(sv));
                    } break;
                    case GpReflectType::UUID:
                    {
                        const auto&         v   = optVal.as<boost::container::string>();
                        std::string_view    sv  = {v.data(), v.size()};
                        propInfo.Value_UUID(dataPtr) = GpUUID::SFromString(sv);
                    } break;
                    case GpReflectType::STRING:
                    {
                        const auto&         v   = optVal.as<boost::container::string>();
                        std::string_view    sv  = {v.data(), v.size()};
                        propInfo.Value_String(dataPtr) = sv;
                    } break;
                    case GpReflectType::BLOB:
                    {
                        const auto&         v   = optVal.as<boost::container::string>();
                        std::string_view    sv  = {v.data(), v.size()};
                        propInfo.Value_BLOB(dataPtr) = StrOps::SToBytesHex(sv);
                    } break;
                    case GpReflectType::ENUM:
                    {
                        const auto&         v   = optVal.as<boost::container::string>();
                        std::string_view    sv  = {v.data(), v.size()};
                        propInfo.Value_Enum(dataPtr).FromString(sv);
                    } break;
                    case GpReflectType::ENUM_FLAGS:
                    {
                        const auto& v = optVal.as<std::vector<boost::container::string>>();
                        GpVector<std::string> vs;
                        for (const auto& s: v)
                        {
                            vs.emplace_back(s);
                        }

                        propInfo.Value_EnumFlags(dataPtr).FromStringArray(vs);
                    } break;
                    case GpReflectType::BOOLEAN:    [[fallthrough]];
                    case GpReflectType::OBJECT:     [[fallthrough]];
                    case GpReflectType::OBJECT_SP:  [[fallthrough]];
                    case GpReflectType::NOT_SET:    [[fallthrough]];
                    default:
                    {
                        THROW_GP("Unsupported type '"_sv + GpReflectType::SToString(propType) + "' of prop '"_sv + propName + "'"_sv);
                    }
                }//switch (propType)
            } else
            {
                const auto& v = optVal.as<std::vector<boost::container::string>>();

                switch (propType)
                {
                    case GpReflectType::S_INT_8:
                    {
                        auto& vec = propInfo.Vec_SInt8(dataPtr); vec.clear();
                        for (const auto& e: v) vec.emplace_back(NumOps::SConvert<s_int_8>(StrOps::SToSI64({e.data(), e.size()})));
                    } break;
                    case GpReflectType::U_INT_8:
                    {
                        auto& vec = propInfo.Vec_UInt8(dataPtr); vec.clear();
                        for (const auto& e: v) vec.emplace_back(NumOps::SConvert<u_int_8>(StrOps::SToUI64({e.data(), e.size()})));
                    } break;
                    case GpReflectType::S_INT_16:
                    {
                        auto& vec = propInfo.Vec_SInt16(dataPtr); vec.clear();
                        for (const auto& e: v) vec.emplace_back(NumOps::SConvert<s_int_16>(StrOps::SToSI64({e.data(), e.size()})));
                    } break;
                    case GpReflectType::U_INT_16:
                    {
                        auto& vec = propInfo.Vec_UInt16(dataPtr); vec.clear();
                        for (const auto& e: v) vec.emplace_back(NumOps::SConvert<u_int_16>(StrOps::SToUI64({e.data(), e.size()})));
                    } break;
                    case GpReflectType::S_INT_32:
                    {
                        auto& vec = propInfo.Vec_SInt32(dataPtr); vec.clear();
                        for (const auto& e: v) vec.emplace_back(NumOps::SConvert<s_int_32>(StrOps::SToSI64({e.data(), e.size()})));
                    } break;
                    case GpReflectType::U_INT_32:
                    {
                        auto& vec = propInfo.Vec_UInt32(dataPtr); vec.clear();
                        for (const auto& e: v) vec.emplace_back(NumOps::SConvert<u_int_32>(StrOps::SToUI64({e.data(), e.size()})));
                    } break;
                    case GpReflectType::S_INT_64:
                    {
                        auto& vec = propInfo.Vec_SInt64(dataPtr); vec.clear();
                        for (const auto& e: v) vec.emplace_back(NumOps::SConvert<s_int_64>(StrOps::SToSI64({e.data(), e.size()})));
                    } break;
                    case GpReflectType::U_INT_64:
                    {
                        auto& vec = propInfo.Vec_UInt64(dataPtr); vec.clear();
                        for (const auto& e: v) vec.emplace_back(NumOps::SConvert<u_int_64>(StrOps::SToUI64({e.data(), e.size()})));
                    } break;
                    case GpReflectType::DOUBLE:
                    {
                        auto& vec = propInfo.Vec_Double(dataPtr); vec.clear();
                        for (const auto& e: v) vec.emplace_back(NumOps::SConvert<double>(StrOps::SToDouble({e.data(), e.size()})));
                    } break;
                    case GpReflectType::FLOAT:
                    {
                        auto& vec = propInfo.Vec_Float(dataPtr); vec.clear();
                        for (const auto& e: v) vec.emplace_back(NumOps::SConvert<float>(StrOps::SToDouble({e.data(), e.size()})));
                    } break;
                    case GpReflectType::UUID:
                    {
                        auto& vec = propInfo.Vec_UUID(dataPtr); vec.clear();
                        for (const auto& e: v) vec.emplace_back(GpUUID::SFromString({e.data(), e.size()}));
                    } break;
                    case GpReflectType::STRING:
                    {
                        auto& vec = propInfo.Vec_String(dataPtr); vec.clear();
                        for (const auto& e: v)
                        {
                            std::string_view sv = {e.data(), e.size()};
                            vec.emplace_back(sv);
                        }
                    } break;
                    case GpReflectType::BLOB:
                    {
                        auto& vec = propInfo.Vec_BLOB(dataPtr); vec.clear();
                        for (const auto& e: v) vec.emplace_back(StrOps::SToBytesHex({e.data(), e.size()}));
                    } break;
                    case GpReflectType::ENUM:       [[fallthrough]];
                    case GpReflectType::ENUM_FLAGS: [[fallthrough]];
                    case GpReflectType::BOOLEAN:    [[fallthrough]];
                    case GpReflectType::OBJECT:     [[fallthrough]];
                    case GpReflectType::OBJECT_SP:  [[fallthrough]];
                    case GpReflectType::NOT_SET:    [[fallthrough]];
                    default:
                    {
                        THROW_GP("Unsupported array of type '"_sv + GpReflectType::SToString(propType) + "' of prop '"_sv + propName + "'"_sv);
                    }
                }
            }
        }
    }
}

}//namespace GPlatform
