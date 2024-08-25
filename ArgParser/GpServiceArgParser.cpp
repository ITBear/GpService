#include "GpServiceArgParser.hpp"
#include <GpCore2/GpUtils/Types/Strings/GpUTF.hpp>

GP_WARNING_PUSH()

#if defined(GP_COMPILER_CLANG) || defined(GP_COMPILER_GCC)
    GP_WARNING_DISABLE(conversion)
    GP_WARNING_DISABLE(shadow)
    GP_WARNING_DISABLE(deprecated-declarations)
    GP_WARNING_DISABLE(deprecated-builtins)
    GP_WARNING_DISABLE(double-promotion)
#endif// #if defined(GP_COMPILER_CLANG) || defined(GP_COMPILER_GCC)

#if defined(GP_COMPILER_MSVC)
#   pragma warning(disable : 4371)
#endif// #if defined(GP_COMPILER_MSVC)

#define BOOST_ALL_DYN_LINK

#include <boost/program_options.hpp>
#include <boost/container/string.hpp>

GP_WARNING_POP()

namespace GPlatform {

GpServiceArgBaseDesc::SP    GpServiceArgParser::SParse
(
    size_t                              aArgc,
    char**                              aArgv,
    const GpServiceArgBaseDescFactory&  aFactory,
    std::string_view                    aDescText
)
{
    std::string descText(aDescText);
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

    GpReflectModel::CSP     modelCSP    = aOut.ReflectModel();
    const GpReflectModel&   model       = modelCSP.Vn();

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
                    aOptDesc.add_options()
                    (
                        std::data(propName),
                        boost::program_options::value<boost::container::string>(),
                        ""
                    );
                } else
                {
                    aOptDesc.add_options()
                    (
                        std::data(propName),
                        boost::program_options::value<std::vector<boost::container::string>>()->multitoken(),
                        ""
                    );
                }
            } break;
            case GpReflectType::ENUM_FLAGS:
            {
                THROW_GP
                (
                    fmt::format
                    (
                        "Unsupported type '{}', property name '{}'",
                        GpReflectType{propType},
                        propName
                    )
                );
            } break;
            case GpReflectType::BOOLEAN:
            {
                if (propContainer == GpReflectContainerType::NO)
                {
                    aOptDesc.add_options()
                    (
                        std::data(propName),
                        boost::program_options::bool_switch()->default_value(false),
                        ""
                    );
                } else
                {
                    THROW_GP
                    (
                        fmt::format
                        (
                            "Unsupported container type '{}', property name '{}'",
                            GpReflectContainerType{propContainer},
                            propName
                        )
                    );
                }
            } break;
            case GpReflectType::OBJECT:    [[fallthrough]];
            case GpReflectType::OBJECT_SP: [[fallthrough]];
            case GpReflectType::NOT_SET:   [[fallthrough]];
            default:
            {
                THROW_GP
                (
                    fmt::format
                    (
                        "Unsupported type '{}', property name '{}'",
                        GpReflectType{propType},
                        propName
                    )
                );
            }
        }
    }
}

void    GpServiceArgParser::SParseOptions
(
    size_t                  aArgc,
    char**                  aArgv,
    const OptDescT&         aOptDesc,
    GpServiceArgBaseDesc&   aOut
)
{
    if (aArgc == 0)
    {
        return;
    }

#if defined(GP_OS_WINDOWS)
    aArgc = NumOps::SSub<size_t>(aArgc, size_t{1});
    aArgv++;

    if (aArgc == 0)
    {
        return;
    }
#endif// #if defined(GP_OS_WINDOWS)

    boost::program_options::variables_map vm;

    try
    {
        boost::program_options::store(boost::program_options::parse_command_line(NumOps::SConvert<int>(aArgc), aArgv, aOptDesc), vm);
        boost::program_options::notify(vm);
    } catch (const std::exception& ex)
    {
        THROW_GP
        (
            fmt::format
            (
                "Failed to parse cmd line arguments: {}",
                ex.what()
            )
        );
    }

    GpReflectModel::CSP     modelCSP    = aOut.ReflectModel();
    const GpReflectModel&   model       = modelCSP.Vn();
    void*                   dataPtr = aOut.ReflectDataPtr();

    for (const GpReflectProp& propInfo: model.Props())
    {
        const std::string                   propName        = std::string(propInfo.Name());
        const GpReflectType::EnumT          propType        = propInfo.Type();
        const GpReflectContainerType::EnumT propContainer   = propInfo.Container();

        if (!vm.count(std::data(propName)))
        {
            continue;
        }

        const auto& optVal = vm[std::data(propName)];

        if (propContainer == GpReflectContainerType::NO)
        {
            switch (propType)
            {
                case GpReflectType::S_INT_8:
                {
                    THROW_COND_GP
                    (
                        !optVal.empty(),
                        [&](){return "Empty value for property '"_sv + propName + "'"_sv;}
                    );

                    const auto&         v   = optVal.as<boost::container::string>();
                    std::string_view    sv(std::data(v), std::size(v));
                    propInfo.Value_SI8(dataPtr) = NumOps::SConvert<s_int_8>(StrOps::SToSI64(sv));
                } break;
                case GpReflectType::U_INT_8:
                {
                    THROW_COND_GP
                    (
                        !optVal.empty(),
                        [&](){return "Empty value for property '"_sv + propName + "'"_sv;}
                    );

                    const auto&         v   = optVal.as<boost::container::string>();
                    std::string_view    sv(std::data(v), std::size(v));
                    propInfo.Value_UI8(dataPtr) = NumOps::SConvert<u_int_8>(StrOps::SToUI64(sv));
                } break;
                case GpReflectType::S_INT_16:
                {
                    THROW_COND_GP
                    (
                        !optVal.empty(),
                        [&](){return "Empty value for property '"_sv + propName + "'"_sv;}
                    );

                    const auto&         v   = optVal.as<boost::container::string>();
                    std::string_view    sv(std::data(v), std::size(v));
                    propInfo.Value_SI16(dataPtr) = NumOps::SConvert<s_int_16>(StrOps::SToSI64(sv));
                } break;
                case GpReflectType::U_INT_16:
                {
                    THROW_COND_GP
                    (
                        !optVal.empty(),
                        [&](){return "Empty value for property '"_sv + propName + "'"_sv;}
                    );

                    const auto&         v   = optVal.as<boost::container::string>();
                    std::string_view    sv(std::data(v), std::size(v));
                    propInfo.Value_UI16(dataPtr) = NumOps::SConvert<u_int_16>(StrOps::SToUI64(sv));
                } break;
                case GpReflectType::S_INT_32:
                {
                    THROW_COND_GP
                    (
                        !optVal.empty(),
                        [&](){return "Empty value for property '"_sv + propName + "'"_sv;}
                    );

                    const auto&         v   = optVal.as<boost::container::string>();
                    std::string_view    sv(std::data(v), std::size(v));
                    propInfo.Value_SI32(dataPtr) = NumOps::SConvert<s_int_32>(StrOps::SToSI64(sv));
                } break;
                case GpReflectType::U_INT_32:
                {
                    THROW_COND_GP
                    (
                        !optVal.empty(),
                        [&](){return "Empty value for property '"_sv + propName + "'"_sv;}
                    );

                    const auto&         v   = optVal.as<boost::container::string>();
                    std::string_view    sv(std::data(v), std::size(v));
                    propInfo.Value_UI32(dataPtr) = NumOps::SConvert<u_int_32>(StrOps::SToUI64(sv));
                } break;
                case GpReflectType::S_INT_64:
                {
                    THROW_COND_GP
                    (
                        !optVal.empty(),
                        [&](){return "Empty value for property '"_sv + propName + "'"_sv;}
                    );

                    const auto&         v   = optVal.as<boost::container::string>();
                    std::string_view    sv(std::data(v), std::size(v));
                    propInfo.Value_SI64(dataPtr) = NumOps::SConvert<s_int_64>(StrOps::SToSI64(sv));
                } break;
                case GpReflectType::U_INT_64:
                {
                    THROW_COND_GP
                    (
                        !optVal.empty(),
                        [&](){return "Empty value for property '"_sv + propName + "'"_sv;}
                    );

                    const auto&         v   = optVal.as<boost::container::string>();
                    std::string_view    sv(std::data(v), std::size(v));
                    propInfo.Value_UI64(dataPtr) = NumOps::SConvert<u_int_64>(StrOps::SToUI64(sv));
                } break;
                case GpReflectType::DOUBLE:
                {
                    THROW_COND_GP
                    (
                        !optVal.empty(),
                        [&](){return "Empty value for property '"_sv + propName + "'"_sv;}
                    );

                    const auto&         v   = optVal.as<boost::container::string>();
                    std::string_view    sv(std::data(v), std::size(v));
                    propInfo.Value_Double(dataPtr) = NumOps::SConvert<double>(StrOps::SToDouble(sv));
                } break;
                case GpReflectType::FLOAT:
                {
                    THROW_COND_GP
                    (
                        !optVal.empty(),
                        [&](){return "Empty value for property '"_sv + propName + "'"_sv;}
                    );

                    const auto&         v   = optVal.as<boost::container::string>();
                    std::string_view    sv(std::data(v), std::size(v));
                    propInfo.Value_Float(dataPtr) = NumOps::SConvert<float>(StrOps::SToDouble(sv));
                } break;
                case GpReflectType::UUID:
                {
                    THROW_COND_GP
                    (
                        !optVal.empty(),
                        [&](){return "Empty value for property '"_sv + propName + "'"_sv;}
                    );

                    const auto&         v   = optVal.as<boost::container::string>();
                    std::string_view    sv(std::data(v), std::size(v));
                    propInfo.Value_UUID(dataPtr) = GpUUID::SFromString(sv);
                } break;
                case GpReflectType::STRING:
                {
                    THROW_COND_GP
                    (
                        !optVal.empty(),
                        [&](){return "Empty value for property '"_sv + propName + "'"_sv;}
                    );

                    const auto&         v   = optVal.as<boost::container::string>();
                    std::string_view    sv(std::data(v), std::size(v));
                    propInfo.Value_String(dataPtr) = sv;
                } break;
                case GpReflectType::BLOB:
                {
                    THROW_COND_GP
                    (
                        !optVal.empty(),
                        [&](){return "Empty value for property '"_sv + propName + "'"_sv;}
                    );

                    const auto&         v   = optVal.as<boost::container::string>();
                    std::string_view    sv(std::data(v), std::size(v));
                    propInfo.Value_BLOB(dataPtr) = StrOps::SToBytesHex(sv);
                } break;
                case GpReflectType::ENUM:
                {
                    THROW_COND_GP
                    (
                        !optVal.empty(),
                        [&](){return "Empty value for property '"_sv + propName + "'"_sv;}
                    );

                    const auto&         v   = optVal.as<boost::container::string>();
                    std::string_view    sv(std::data(v), std::size(v));
                    propInfo.Value_Enum(dataPtr).FromString(GpUTF::SToUpper(sv));
                } break;
                case GpReflectType::ENUM_FLAGS:
                {
                    THROW_COND_GP
                    (
                        !optVal.empty(),
                        [&](){return "Empty value for property '"_sv + propName + "'"_sv;}
                    );

                    const auto& v = optVal.as<std::vector<boost::container::string>>();
                    std::vector<std::string> vs;
                    for (const auto& s: v)
                    {
                        vs.emplace_back(s);
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
                    THROW_GP("Unsupported type '"_sv + GpReflectType::SToString(propType) + "' of prop '"_sv + propName + "'"_sv);
                }
            }//switch (propType)
        } else if (propContainer == GpReflectContainerType::VECTOR)
        {
            const auto& v = optVal.as<std::vector<boost::container::string>>();

            switch (propType)
            {
                case GpReflectType::S_INT_8:
                {
                    auto& vec = propInfo.Vec_SI8(dataPtr); vec.clear();
                    for (const auto& e: v)
                    {
                        vec.emplace_back(NumOps::SConvert<s_int_8>(StrOps::SToSI64(std::string_view(std::data(e), std::size(e)))));
                    }
                } break;
                case GpReflectType::U_INT_8:
                {
                    auto& vec = propInfo.Vec_UI8(dataPtr); vec.clear();
                    for (const auto& e: v)
                    {
                        vec.emplace_back(NumOps::SConvert<u_int_8>(StrOps::SToUI64(std::string_view(std::data(e), std::size(e)))));
                    }
                } break;
                case GpReflectType::S_INT_16:
                {
                    auto& vec = propInfo.Vec_SI16(dataPtr); vec.clear();
                    for (const auto& e: v)
                    {
                        vec.emplace_back(NumOps::SConvert<s_int_16>(StrOps::SToSI64(std::string_view(std::data(e), std::size(e)))));
                    }
                } break;
                case GpReflectType::U_INT_16:
                {
                    auto& vec = propInfo.Vec_UI16(dataPtr); vec.clear();
                    for (const auto& e: v)
                    {
                        vec.emplace_back(NumOps::SConvert<u_int_16>(StrOps::SToUI64(std::string_view(std::data(e), std::size(e)))));
                    }
                } break;
                case GpReflectType::S_INT_32:
                {
                    auto& vec = propInfo.Vec_SI32(dataPtr); vec.clear();
                    for (const auto& e: v)
                    {
                        vec.emplace_back(NumOps::SConvert<s_int_32>(StrOps::SToSI64(std::string_view(std::data(e), std::size(e)))));
                    }
                } break;
                case GpReflectType::U_INT_32:
                {
                    auto& vec = propInfo.Vec_UI32(dataPtr); vec.clear();
                    for (const auto& e: v)
                    {
                        vec.emplace_back(NumOps::SConvert<u_int_32>(StrOps::SToUI64(std::string_view(std::data(e), std::size(e)))));
                    }
                } break;
                case GpReflectType::S_INT_64:
                {
                    auto& vec = propInfo.Vec_SI64(dataPtr); vec.clear();
                    for (const auto& e: v)
                    {
                        vec.emplace_back(NumOps::SConvert<s_int_64>(StrOps::SToSI64(std::string_view(std::data(e), std::size(e)))));
                    }
                } break;
                case GpReflectType::U_INT_64:
                {
                    auto& vec = propInfo.Vec_UI64(dataPtr); vec.clear();
                    for (const auto& e: v)
                    {
                        vec.emplace_back(NumOps::SConvert<u_int_64>(StrOps::SToUI64(std::string_view(std::data(e), std::size(e)))));
                    }
                } break;
                case GpReflectType::DOUBLE:
                {
                    auto& vec = propInfo.Vec_Double(dataPtr); vec.clear();
                    for (const auto& e: v)
                    {
                        vec.emplace_back(NumOps::SConvert<double>(StrOps::SToDouble(std::string_view(std::data(e), std::size(e)))));
                    }
                } break;
                case GpReflectType::FLOAT:
                {
                    auto& vec = propInfo.Vec_Float(dataPtr); vec.clear();
                    for (const auto& e: v)
                    {
                        vec.emplace_back(NumOps::SConvert<float>(StrOps::SToDouble(std::string_view(std::data(e), std::size(e)))));
                    }
                } break;
                case GpReflectType::UUID:
                {
                    auto& vec = propInfo.Vec_UUID(dataPtr); vec.clear();
                    for (const auto& e: v)
                    {
                        vec.emplace_back(GpUUID::SFromString(std::string_view(std::data(e), std::size(e))));
                    }
                } break;
                case GpReflectType::STRING:
                {
                    auto& vec = propInfo.Vec_String(dataPtr); vec.clear();
                    for (const auto& e: v)
                    {
                        std::string_view sv(std::data(e), std::size(e));
                        vec.emplace_back(sv);
                    }
                } break;
                case GpReflectType::BLOB:
                {
                    auto& vec = propInfo.Vec_BLOB(dataPtr); vec.clear();
                    for (const auto& e: v)
                    {
                        vec.emplace_back(StrOps::SToBytesHex(std::string_view(std::data(e), std::size(e))));
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
                        fmt::format
                        (
                            "Unsupported array of type '{}' of prop '{}'",
                            GpReflectType{propType},
                            propName
                        )
                    );
                }
            }
        } else
        {
            THROW_GP
            (
                fmt::format
                (
                    "Unsupported conatainer type '{}'",
                    GpReflectContainerType{propContainer}
                )
            );
        }
    }
}

}// namespace GPlatform
