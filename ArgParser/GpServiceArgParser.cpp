#include "GpServiceArgParser.hpp"

GP_WARNING_PUSH()
GP_WARNING_DISABLE(conversion)
GP_WARNING_DISABLE(shadow)
GP_WARNING_DISABLE(deprecated-declarations)
GP_WARNING_DISABLE(deprecated-builtins)
GP_WARNING_DISABLE(double-promotion)

#include <boost/program_options.hpp>
#include <boost/container/string.hpp>

GP_WARNING_POP()

namespace GPlatform {

GpServiceArgBaseDesc::SP    GpServiceArgParser::SParse
(
    const size_t                        aArgc,
    char**                              aArgv,
    const GpServiceArgBaseDescFactory&  aFactory,
    std::u8string_view                  aDescText
)
{
    std::string descText(GpUTF::S_As_STR(aDescText));
    OptDescT optDesc(std::move(descText));

    GpServiceArgBaseDesc::SP descSP = aFactory.NewInstance(aArgc, aArgv);

    SFillOptions(optDesc, descSP.V());
    SParseOptions(aArgc, aArgv, optDesc, descSP.V());

    return descSP;
}

void    GpServiceArgParser::SFillOptions
(
    OptDescT&                   aOptDesc,
    const GpServiceArgBaseDesc& aOut
)
{
    aOptDesc.add_options()("help", "Show help message");

    const GpReflectModel& model = aOut.ReflectModel();

    for (const GpReflectProp& propInfo: model.Props())
    {
        const std::u8string                 propName        = std::u8string(propInfo.Name());
        const GpReflectType::EnumT          propType        = propInfo.Type();
        const GpReflectContainerType::EnumT propContainer   = propInfo.Container();

        THROW_COND_GP
        (
               (propContainer == GpReflectContainerType::NO)
            || (propContainer == GpReflectContainerType::VECTOR),
            [&](){return u8"Property '"_sv + propName + u8"' container must be NO or VECTOR"_sv;}
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
                    aOptDesc.add_options()
                    (
                        GpUTF::S_As_STR(propName).data(),
                        boost::program_options::value<boost::container::string>(),
                        ""
                    );
                } else
                {
                    aOptDesc.add_options()
                    (
                        GpUTF::S_As_STR(propName).data(),
                        boost::program_options::value<std::vector<boost::container::string>>()->multitoken(),
                        ""
                    );
                }
            } break;
            case GpReflectType::ENUM_FLAGS:
            {
                if (propContainer == GpReflectContainerType::NO)
                {
                    aOptDesc.add_options()
                    (
                        GpUTF::S_As_STR(propName).data(),
                        ""
                    );
                } else
                {
                    THROW_GP
                    (
                          u8"Unsupported container type '"_sv + GpReflectContainerType::SToString(propContainer)
                        + u8"', property name '"_sv + propName + u8"'"_sv
                    );
                }
            } break;
            case GpReflectType::BOOLEAN:
            {
                if (propContainer == GpReflectContainerType::NO)
                {
                    aOptDesc.add_options()(GpUTF::S_As_STR(propName).data(), "");

                    aOptDesc.add_options()
                    (
                        GpUTF::S_As_STR(propName).data(),
                        boost::program_options::value<std::vector<boost::container::string>>()->multitoken(),
                        ""
                    );
                } else
                {
                    THROW_GP
                    (
                          u8"Unsupported container type '"_sv + GpReflectContainerType::SToString(propContainer)
                        + u8"', property name '"_sv + propName + u8"'"_sv
                    );
                }
            } break;
            case GpReflectType::OBJECT:    [[fallthrough]];
            case GpReflectType::OBJECT_SP: [[fallthrough]];
            case GpReflectType::NOT_SET:   [[fallthrough]];
            default:
            {
                THROW_GP(u8"Unsupported type '"_sv + GpReflectType::SToString(propType) + u8"', property name '"_sv + propName + u8"'"_sv);
            }
        }
    }
}

void    GpServiceArgParser::SParseOptions
(
    const size_t            aArgc,
    char**                  aArgv,
    const OptDescT&         aOptDesc,
    GpServiceArgBaseDesc&   aOut
)
{
    boost::program_options::variables_map vm;

    try
    {
        boost::program_options::store(boost::program_options::parse_command_line(NumOps::SConvert<int>(aArgc), aArgv, aOptDesc), vm);
        boost::program_options::notify(vm);
    } catch (const std::exception& ex)
    {
        THROW_GP(u8"Failed to parse cmd line arguments: "_sv + ex.what());
    }

    const GpReflectModel&   model   = aOut.ReflectModel();
    void*                   dataPtr = aOut.ReflectDataPtr();

    for (const GpReflectProp& propInfo: model.Props())
    {
        const std::u8string                 propName        = std::u8string(propInfo.Name());
        const GpReflectType::EnumT          propType        = propInfo.Type();
        const GpReflectContainerType::EnumT propContainer   = propInfo.Container();

        if (!vm.count(GpUTF::S_As_STR(propName).data()))
        {
            continue;
        }

        const auto& optVal = vm[GpUTF::S_As_STR(propName).data()];

        if (propContainer == GpReflectContainerType::NO)
        {
            switch (propType)
            {
                case GpReflectType::S_INT_8:
                {
                    THROW_COND_GP
                    (
                        !optVal.empty(),
                        [&](){return u8"Empty value for property '"_sv + propName + u8"'"_sv;}
                    );

                    const auto&         v   = optVal.as<boost::container::string>();
                    std::u8string_view  sv  = GpUTF::S_As_UTF8(v.data(), v.size());
                    propInfo.Value_SInt8(dataPtr) = NumOps::SConvert<s_int_8>(StrOps::SToSI64(sv));
                } break;
                case GpReflectType::U_INT_8:
                {
                    THROW_COND_GP
                    (
                        !optVal.empty(),
                        [&](){return u8"Empty value for property '"_sv + propName + u8"'"_sv;}
                    );

                    const auto&         v   = optVal.as<boost::container::string>();
                    std::u8string_view  sv  = GpUTF::S_As_UTF8(v.data(), v.size());
                    propInfo.Value_UInt8(dataPtr) = NumOps::SConvert<u_int_8>(StrOps::SToUI64(sv));
                } break;
                case GpReflectType::S_INT_16:
                {
                    THROW_COND_GP
                    (
                        !optVal.empty(),
                        [&](){return u8"Empty value for property '"_sv + propName + u8"'"_sv;}
                    );

                    const auto&         v   = optVal.as<boost::container::string>();
                    std::u8string_view  sv  = GpUTF::S_As_UTF8(v.data(), v.size());
                    propInfo.Value_SInt16(dataPtr) = NumOps::SConvert<s_int_16>(StrOps::SToSI64(sv));
                } break;
                case GpReflectType::U_INT_16:
                {
                    THROW_COND_GP
                    (
                        !optVal.empty(),
                        [&](){return u8"Empty value for property '"_sv + propName + u8"'"_sv;}
                    );

                    const auto&         v   = optVal.as<boost::container::string>();
                    std::u8string_view  sv  = GpUTF::S_As_UTF8(v.data(), v.size());
                    propInfo.Value_UInt16(dataPtr) = NumOps::SConvert<u_int_16>(StrOps::SToUI64(sv));
                } break;
                case GpReflectType::S_INT_32:
                {
                    THROW_COND_GP
                    (
                        !optVal.empty(),
                        [&](){return u8"Empty value for property '"_sv + propName + u8"'"_sv;}
                    );

                    const auto&         v   = optVal.as<boost::container::string>();
                    std::u8string_view  sv  = GpUTF::S_As_UTF8(v.data(), v.size());
                    propInfo.Value_SInt32(dataPtr) = NumOps::SConvert<s_int_32>(StrOps::SToSI64(sv));
                } break;
                case GpReflectType::U_INT_32:
                {
                    THROW_COND_GP
                    (
                        !optVal.empty(),
                        [&](){return u8"Empty value for property '"_sv + propName + u8"'"_sv;}
                    );

                    const auto&         v   = optVal.as<boost::container::string>();
                    std::u8string_view  sv  = GpUTF::S_As_UTF8(v.data(), v.size());
                    propInfo.Value_UInt32(dataPtr) = NumOps::SConvert<u_int_32>(StrOps::SToUI64(sv));
                } break;
                case GpReflectType::S_INT_64:
                {
                    THROW_COND_GP
                    (
                        !optVal.empty(),
                        [&](){return u8"Empty value for property '"_sv + propName + u8"'"_sv;}
                    );

                    const auto&         v   = optVal.as<boost::container::string>();
                    std::u8string_view  sv  = GpUTF::S_As_UTF8(v.data(), v.size());
                    propInfo.Value_SInt64(dataPtr) = NumOps::SConvert<s_int_64>(StrOps::SToSI64(sv));
                } break;
                case GpReflectType::U_INT_64:
                {
                    THROW_COND_GP
                    (
                        !optVal.empty(),
                        [&](){return u8"Empty value for property '"_sv + propName + u8"'"_sv;}
                    );

                    const auto&         v   = optVal.as<boost::container::string>();
                    std::u8string_view  sv  = GpUTF::S_As_UTF8(v.data(), v.size());
                    propInfo.Value_UInt64(dataPtr) = NumOps::SConvert<u_int_64>(StrOps::SToUI64(sv));
                } break;
                case GpReflectType::DOUBLE:
                {
                    THROW_COND_GP
                    (
                        !optVal.empty(),
                        [&](){return u8"Empty value for property '"_sv + propName + u8"'"_sv;}
                    );

                    const auto&         v   = optVal.as<boost::container::string>();
                    std::u8string_view  sv  = GpUTF::S_As_UTF8(v.data(), v.size());
                    propInfo.Value_Double(dataPtr) = NumOps::SConvert<double>(StrOps::SToDouble(sv));
                } break;
                case GpReflectType::FLOAT:
                {
                    THROW_COND_GP
                    (
                        !optVal.empty(),
                        [&](){return u8"Empty value for property '"_sv + propName + u8"'"_sv;}
                    );

                    const auto&         v   = optVal.as<boost::container::string>();
                    std::u8string_view  sv  = GpUTF::S_As_UTF8(v.data(), v.size());
                    propInfo.Value_Float(dataPtr) = NumOps::SConvert<float>(StrOps::SToDouble(sv));
                } break;
                case GpReflectType::UUID:
                {
                    THROW_COND_GP
                    (
                        !optVal.empty(),
                        [&](){return u8"Empty value for property '"_sv + propName + u8"'"_sv;}
                    );

                    const auto&         v   = optVal.as<boost::container::string>();
                    std::u8string_view  sv  = GpUTF::S_As_UTF8(v.data(), v.size());
                    propInfo.Value_UUID(dataPtr) = GpUUID::SFromString(sv);
                } break;
                case GpReflectType::STRING:
                {
                    THROW_COND_GP
                    (
                        !optVal.empty(),
                        [&](){return u8"Empty value for property '"_sv + propName + u8"'"_sv;}
                    );

                    const auto&         v   = optVal.as<boost::container::string>();
                    std::u8string_view  sv  = GpUTF::S_As_UTF8(v.data(), v.size());
                    propInfo.Value_String(dataPtr) = sv;
                } break;
                case GpReflectType::BLOB:
                {
                    THROW_COND_GP
                    (
                        !optVal.empty(),
                        [&](){return u8"Empty value for property '"_sv + propName + u8"'"_sv;}
                    );

                    const auto&         v   = optVal.as<boost::container::string>();
                    std::u8string_view  sv  = GpUTF::S_As_UTF8(v.data(), v.size());
                    propInfo.Value_BLOB(dataPtr) = StrOps::SToBytesHex(sv);
                } break;
                case GpReflectType::ENUM:
                {
                    THROW_COND_GP
                    (
                        !optVal.empty(),
                        [&](){return u8"Empty value for property '"_sv + propName + u8"'"_sv;}
                    );

                    const auto&         v   = optVal.as<boost::container::string>();
                    std::u8string_view  sv  = GpUTF::S_As_UTF8(v.data(), v.size());
                    propInfo.Value_Enum(dataPtr).FromString(GpUTF::SToUpper(sv));
                } break;
                case GpReflectType::ENUM_FLAGS:
                {
                    THROW_COND_GP
                    (
                        !optVal.empty(),
                        [&](){return u8"Empty value for property '"_sv + propName + u8"'"_sv;}
                    );

                    const auto& v = optVal.as<std::vector<boost::container::string>>();
                    std::vector<std::u8string> vs;
                    for (const auto& s: v)
                    {
                        vs.emplace_back(GpUTF::S_As_UTF8(s));
                    }

                    propInfo.Value_EnumFlags(dataPtr).FromStringArray(vs);
                } break;
                case GpReflectType::BOOLEAN:
                {
                    propInfo.Value_Bool(dataPtr) = true;
                } break;
                case GpReflectType::OBJECT:     [[fallthrough]];
                case GpReflectType::OBJECT_SP:  [[fallthrough]];
                case GpReflectType::NOT_SET:    [[fallthrough]];
                default:
                {
                    THROW_GP(u8"Unsupported type '"_sv + GpReflectType::SToString(propType) + u8"' of prop '"_sv + propName + u8"'"_sv);
                }
            }//switch (propType)
        } else if (propContainer == GpReflectContainerType::VECTOR)
        {
            const auto& v = optVal.as<std::vector<boost::container::string>>();

            switch (propType)
            {
                case GpReflectType::S_INT_8:
                {
                    auto& vec = propInfo.Vec_SInt8(dataPtr); vec.clear();
                    for (const auto& e: v)
                    {
                        vec.emplace_back(NumOps::SConvert<s_int_8>(StrOps::SToSI64(GpUTF::S_As_UTF8(e.data(), e.size()))));
                    }
                } break;
                case GpReflectType::U_INT_8:
                {
                    auto& vec = propInfo.Vec_UInt8(dataPtr); vec.clear();
                    for (const auto& e: v)
                    {
                        vec.emplace_back(NumOps::SConvert<u_int_8>(StrOps::SToUI64(GpUTF::S_As_UTF8(e.data(), e.size()))));
                    }
                } break;
                case GpReflectType::S_INT_16:
                {
                    auto& vec = propInfo.Vec_SInt16(dataPtr); vec.clear();
                    for (const auto& e: v)
                    {
                        vec.emplace_back(NumOps::SConvert<s_int_16>(StrOps::SToSI64(GpUTF::S_As_UTF8(e.data(), e.size()))));
                    }
                } break;
                case GpReflectType::U_INT_16:
                {
                    auto& vec = propInfo.Vec_UInt16(dataPtr); vec.clear();
                    for (const auto& e: v)
                    {
                        vec.emplace_back(NumOps::SConvert<u_int_16>(StrOps::SToUI64(GpUTF::S_As_UTF8(e.data(), e.size()))));
                    }
                } break;
                case GpReflectType::S_INT_32:
                {
                    auto& vec = propInfo.Vec_SInt32(dataPtr); vec.clear();
                    for (const auto& e: v)
                    {
                        vec.emplace_back(NumOps::SConvert<s_int_32>(StrOps::SToSI64(GpUTF::S_As_UTF8(e.data(), e.size()))));
                    }
                } break;
                case GpReflectType::U_INT_32:
                {
                    auto& vec = propInfo.Vec_UInt32(dataPtr); vec.clear();
                    for (const auto& e: v)
                    {
                        vec.emplace_back(NumOps::SConvert<u_int_32>(StrOps::SToUI64(GpUTF::S_As_UTF8(e.data(), e.size()))));
                    }
                } break;
                case GpReflectType::S_INT_64:
                {
                    auto& vec = propInfo.Vec_SInt64(dataPtr); vec.clear();
                    for (const auto& e: v)
                    {
                        vec.emplace_back(NumOps::SConvert<s_int_64>(StrOps::SToSI64(GpUTF::S_As_UTF8(e.data(), e.size()))));
                    }
                } break;
                case GpReflectType::U_INT_64:
                {
                    auto& vec = propInfo.Vec_UInt64(dataPtr); vec.clear();
                    for (const auto& e: v)
                    {
                        vec.emplace_back(NumOps::SConvert<u_int_64>(StrOps::SToUI64(GpUTF::S_As_UTF8(e.data(), e.size()))));
                    }
                } break;
                case GpReflectType::DOUBLE:
                {
                    auto& vec = propInfo.Vec_Double(dataPtr); vec.clear();
                    for (const auto& e: v)
                    {
                        vec.emplace_back(NumOps::SConvert<double>(StrOps::SToDouble(GpUTF::S_As_UTF8(e.data(), e.size()))));
                    }
                } break;
                case GpReflectType::FLOAT:
                {
                    auto& vec = propInfo.Vec_Float(dataPtr); vec.clear();
                    for (const auto& e: v)
                    {
                        vec.emplace_back(NumOps::SConvert<float>(StrOps::SToDouble(GpUTF::S_As_UTF8(e.data(), e.size()))));
                    }
                } break;
                case GpReflectType::UUID:
                {
                    auto& vec = propInfo.Vec_UUID(dataPtr); vec.clear();
                    for (const auto& e: v)
                    {
                        vec.emplace_back(GpUUID::SFromString(GpUTF::S_As_UTF8(e.data(), e.size())));
                    }
                } break;
                case GpReflectType::STRING:
                {
                    auto& vec = propInfo.Vec_String(dataPtr); vec.clear();
                    for (const auto& e: v)
                    {
                        std::u8string_view sv = GpUTF::S_As_UTF8(e.data(), e.size());
                        vec.emplace_back(sv);
                    }
                } break;
                case GpReflectType::BLOB:
                {
                    auto& vec = propInfo.Vec_BLOB(dataPtr); vec.clear();
                    for (const auto& e: v)
                    {
                        vec.emplace_back(StrOps::SToBytesHex(GpUTF::S_As_UTF8(e.data(), e.size())));
                    }
                } break;
                case GpReflectType::ENUM:       [[fallthrough]];
                case GpReflectType::ENUM_FLAGS: [[fallthrough]];
                case GpReflectType::BOOLEAN:    [[fallthrough]];
                case GpReflectType::OBJECT:     [[fallthrough]];
                case GpReflectType::OBJECT_SP:  [[fallthrough]];
                case GpReflectType::NOT_SET:    [[fallthrough]];
                default:
                {
                    THROW_GP
                    (
                          u8"Unsupported array of type '"_sv + GpReflectType::SToString(propType)
                        + u8"' of prop '"_sv + propName + u8"'"_sv
                    );
                }
            }
        } else
        {
            THROW_GP(u8"Unsupported conatainer type '"_sv + GpReflectContainerType::SToString(propContainer));
        }
    }
}

}//namespace GPlatform
