#include <GpService/ArgParser/GpServiceArgParser.hpp>
#include <GpCore2/GpUtils/Types/Strings/GpUTF.hpp>
#include <GpCore2/Config/IncludeExt/boost_flat_set.hpp>
#include <GpCore2/GpUtils/Types/Strings/GpStringOps.hpp>

namespace GPlatform {

GpServiceArgParser::ResT    GpServiceArgParser::SParse
(
    size_t                              aArgc,
    const char* const                   aArgv[],
    const GpServiceArgBaseDescFactory&  aFactory
)
{
    auto [descSP, isEnableUnknownArguments] = aFactory.NewInstance(aArgc, aArgv);

    GpArgParser argParser;

    if (isEnableUnknownArguments)
    {
        argParser.EnableUnknownArguments();
    }

    SInitArgParser(descSP.V(), argParser);
    GpArgParserRes::SP argParserResSP = SParse(aArgc, aArgv, argParser, descSP.V());

    return {descSP, argParserResSP};
}

void    GpServiceArgParser::SInitArgParser
(
    const GpServiceArgBaseDesc& aArgBaseDesc,
    GpArgParser&                aArgParser
)
{
    GpReflectModel::CSP     modelCSP    = aArgBaseDesc.ReflectModel();
    const GpReflectModel&   model       = modelCSP.Vn();

    for (const GpReflectProp& propInfo: model.Props())
    {
        auto& builder = aArgParser.NextArgument();

        builder.AddName(propInfo.Name());

        if (propInfo.Type() == GpReflectType::BOOLEAN)
        {
            builder.DefaultValue("T");
        }

        builder.Done();
    }
}

GpArgParserRes::SP  GpServiceArgParser::SParse
(
    size_t                  aArgc,
    const char* const       aArgv[],
    const GpArgParser&      aArgParser,
    GpServiceArgBaseDesc&   aArgBaseDescOut
)
{
    if (aArgc == 0)
    {
        return MakeSP<GpArgParserRes>();
    }

//#if defined(GP_OS_WINDOWS)
//  aArgc = NumOps::SSub<size_t>(aArgc, size_t{1});
//  aArgv++;
//
//  if (aArgc == 0)
//  {
//      return;
//  }
//#endif// #if defined(GP_OS_WINDOWS)

    GpArgParserRes::SP      argParserResSP  = aArgParser.Parse(aArgc, aArgv);
    const GpArgParserRes&   argParserRes    = argParserResSP.V();
    GpReflectModel::CSP     modelCSP        = aArgBaseDescOut.ReflectModel();
    const GpReflectModel&   model           = modelCSP.Vn();
    void*                   dataPtr         = aArgBaseDescOut.ReflectDataPtr();

    for (const GpReflectProp& propInfo: model.Props())
    {
        const std::string_view              propName        = propInfo.Name();
        const GpReflectType::EnumT          propType        = propInfo.Type();
        const GpReflectContainerType::EnumT propContainer   = propInfo.Container();

        // Try to find argument by name
        auto argIter = std::find_if
        (
            argParserRes.Arguments().begin(),
            argParserRes.Arguments().end(),
            [propName](const auto& aValue)
            {
                return aValue.V().Names().count(propName) > 0;
            }
        );

        if (argIter == argParserRes.Arguments().end())
        {
            continue;
        }

        const auto&     arg             = argIter->V();
        const auto&     argValues       = arg.Values();
        const size_t    argValuesCount  = std::size(argValues);

        if (propContainer == GpReflectContainerType::NO)
        {
            std::string_view argValue;

            // Check values count
            if (argValuesCount == 0)
            {
                // Try to get default value
                argValue = arg.DefaultValue();

                if (arg.DefaultValue().empty())
                {
                    THROW_GP
                    (
                        fmt::format
                        (
                            "Empty value for property '{}'",
                            propName
                        )
                    );
                }
            } else if (argValuesCount > 1)
            {
                THROW_GP
                (
                    fmt::format
                    (
                        "Multiple values for property '{}'",
                        propName
                    )
                );
            } else
            {
                argValue = std::get<0>(argValues.at(0));
            }

            switch (propType)
            {
                case GpReflectType::S_INT_8:
                {
                    propInfo.Value_SI8(dataPtr) = NumOps::SConvert<s_int_8>(StrOps::SToSI64(argValue));
                } break;
                case GpReflectType::U_INT_8:
                {
                    propInfo.Value_UI8(dataPtr) = NumOps::SConvert<u_int_8>(StrOps::SToUI64(argValue));
                } break;
                case GpReflectType::S_INT_16:
                {
                    propInfo.Value_SI16(dataPtr) = NumOps::SConvert<s_int_16>(StrOps::SToSI64(argValue));
                } break;
                case GpReflectType::U_INT_16:
                {
                    propInfo.Value_UI16(dataPtr) = NumOps::SConvert<u_int_16>(StrOps::SToUI64(argValue));
                } break;
                case GpReflectType::S_INT_32:
                {
                    propInfo.Value_SI32(dataPtr) = NumOps::SConvert<s_int_32>(StrOps::SToSI64(argValue));
                } break;
                case GpReflectType::U_INT_32:
                {
                    propInfo.Value_UI32(dataPtr) = NumOps::SConvert<u_int_32>(StrOps::SToUI64(argValue));
                } break;
                case GpReflectType::S_INT_64:
                {
                    propInfo.Value_SI64(dataPtr) = NumOps::SConvert<s_int_64>(StrOps::SToSI64(argValue));
                } break;
                case GpReflectType::U_INT_64:
                {
                    propInfo.Value_UI64(dataPtr) = NumOps::SConvert<u_int_64>(StrOps::SToUI64(argValue));
                } break;
                case GpReflectType::DOUBLE:
                {
                    propInfo.Value_Double(dataPtr) = NumOps::SConvert<double>(StrOps::SToDouble(argValue));
                } break;
                case GpReflectType::FLOAT:
                {
                    propInfo.Value_Float(dataPtr) = NumOps::SConvert<float>(StrOps::SToDouble(argValue));
                } break;
                case GpReflectType::UUID:
                {
                    propInfo.Value_UUID(dataPtr) = GpUUID::SFromString(argValue);
                } break;
                case GpReflectType::STRING:
                {
                    propInfo.Value_String(dataPtr) = argValue;
                } break;
                case GpReflectType::BLOB:
                {
                    propInfo.Value_BLOB(dataPtr) = StrOps::SToBytesHex(argValue);
                } break;
                case GpReflectType::ENUM:
                {
                    propInfo.Value_Enum(dataPtr).FromString(GpUTF::SToUpper(argValue));
                } break;
                case GpReflectType::BOOLEAN:
                {
                    const std::string val = GpUTF::SToUpper(argValue);
                    bool& propRef = propInfo.Value_Bool(dataPtr);

                    if ((val == "T"_sv) || (val == "TRUE"_sv) || (val == "1"_sv) || (val == "Y"_sv) || (val == "YES"_sv))
                    {
                        propRef = true;
                    } else if ((val == "F"_sv) || (val == "FALSE"_sv) || (val == "0"_sv) || (val == "N"_sv) || (val == "NO"_sv))
                    {
                        propRef = false;
                    } else
                    {
                        THROW_GP
                        (
                            fmt::format
                            (
                                "Failed to convert value '{}' to boolean for property '{}'",
                                argValue,
                                propName
                            )
                        );
                    }
                } break;
                case GpReflectType::ENUM_FLAGS: [[fallthrough]];
                case GpReflectType::OBJECT:     [[fallthrough]];
                case GpReflectType::OBJECT_SP:  [[fallthrough]];
                case GpReflectType::NOT_SET:    [[fallthrough]];
                default:
                {
                    THROW_GP
                    (
                        fmt::format
                        (
                            "Unsupported type '{}' of prop '{}'",
                            GpReflectType{propType},
                            propName
                        )
                    );
                }
            }//switch (propType)
        } else if (propContainer == GpReflectContainerType::VECTOR)
        {
            switch (propType)
            {
                case GpReflectType::S_INT_8:
                {
                    auto& vec = propInfo.Vec_SI8(dataPtr);
                    vec.clear();

                    for (const auto& [e,_]: argValues)
                    {
                        vec.emplace_back(NumOps::SConvert<s_int_8>(StrOps::SToSI64(e)));
                    }
                } break;
                case GpReflectType::U_INT_8:
                {
                    auto& vec = propInfo.Vec_UI8(dataPtr);
                    vec.clear();

                    for (const auto& [e,_]: argValues)
                    {
                        vec.emplace_back(NumOps::SConvert<u_int_8>(StrOps::SToUI64(e)));
                    }
                } break;
                case GpReflectType::S_INT_16:
                {
                    auto& vec = propInfo.Vec_SI16(dataPtr); vec.clear();
                    vec.clear();

                    for (const auto& [e,_]: argValues)
                    {
                        vec.emplace_back(NumOps::SConvert<s_int_16>(StrOps::SToSI64(e)));
                    }
                } break;
                case GpReflectType::U_INT_16:
                {
                    auto& vec = propInfo.Vec_UI16(dataPtr); vec.clear();
                    vec.clear();

                    for (const auto& [e,_]: argValues)
                    {
                        vec.emplace_back(NumOps::SConvert<u_int_16>(StrOps::SToUI64(e)));
                    }
                } break;
                case GpReflectType::S_INT_32:
                {
                    auto& vec = propInfo.Vec_SI32(dataPtr); vec.clear();
                    vec.clear();

                    for (const auto& [e,_]: argValues)
                    {
                        vec.emplace_back(NumOps::SConvert<s_int_32>(StrOps::SToSI64(e)));
                    }
                } break;
                case GpReflectType::U_INT_32:
                {
                    auto& vec = propInfo.Vec_UI32(dataPtr); vec.clear();
                    vec.clear();

                    for (const auto& [e,_]: argValues)
                    {
                        vec.emplace_back(NumOps::SConvert<u_int_32>(StrOps::SToUI64(e)));
                    }
                } break;
                case GpReflectType::S_INT_64:
                {
                    auto& vec = propInfo.Vec_SI64(dataPtr); vec.clear();
                    vec.clear();

                    for (const auto& [e,_]: argValues)
                    {
                        vec.emplace_back(NumOps::SConvert<s_int_64>(StrOps::SToSI64(e)));
                    }
                } break;
                case GpReflectType::U_INT_64:
                {
                    auto& vec = propInfo.Vec_UI64(dataPtr); vec.clear();
                    vec.clear();

                    for (const auto& [e,_]: argValues)
                    {
                        vec.emplace_back(NumOps::SConvert<u_int_64>(StrOps::SToUI64(e)));
                    }
                } break;
                case GpReflectType::DOUBLE:
                {
                    auto& vec = propInfo.Vec_Double(dataPtr); vec.clear();
                    vec.clear();

                    for (const auto& [e,_]: argValues)
                    {
                        vec.emplace_back(NumOps::SConvert<double>(StrOps::SToDouble(e)));
                    }
                } break;
                case GpReflectType::FLOAT:
                {
                    auto& vec = propInfo.Vec_Float(dataPtr); vec.clear();
                    vec.clear();

                    for (const auto& [e,_]: argValues)
                    {
                        vec.emplace_back(NumOps::SConvert<float>(StrOps::SToDouble(e)));
                    }
                } break;
                case GpReflectType::UUID:
                {
                    auto& vec = propInfo.Vec_UUID(dataPtr); vec.clear();
                    vec.clear();

                    for (const auto& [e,_]: argValues)
                    {
                        vec.emplace_back(GpUUID::SFromString(e));
                    }
                } break;
                case GpReflectType::STRING:
                {
                    auto& vec = propInfo.Vec_String(dataPtr); vec.clear();
                    vec.clear();

                    for (const auto& [e,_]: argValues)
                    {
                        vec.emplace_back(e);
                    }
                } break;
                case GpReflectType::BLOB:
                {
                    auto& vec = propInfo.Vec_BLOB(dataPtr); vec.clear();
                    vec.clear();

                    for (const auto& [e,_]: argValues)
                    {
                        vec.emplace_back(StrOps::SToBytesHex(e));
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
                            "Unsupported type '{}' of prop '{}'",
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

    return argParserResSP;
}

}// namespace GPlatform
