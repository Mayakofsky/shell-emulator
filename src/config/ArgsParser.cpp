#include "ArgsParser.hpp"

#include <string_view>

namespace
{
std::string argumentError(std::string_view detail)
{
    return "shell: argument error: " + std::string(detail);
}
}

ArgsParseResult ArgsParser::parse(const std::vector<std::string>& arguments) const
{
    ArgsParseResult result;
    for (std::size_t index = 0; index < arguments.size(); ++index)
    {
        const std::string& argument = arguments[index];
        if (!argument.starts_with("--"))
        {
            result.success = false;
            result.error = argumentError("unknown parameter: " + argument);
            return result;
        }

        const std::size_t equalPosition = argument.find('=');
        const std::string name = argument.substr(2, equalPosition - 2);
        if (!isSupportedName(name))
        {
            result.success = false;
            result.error = argumentError("unknown parameter: " + name);
            return result;
        }

        std::string value;
        if (equalPosition != std::string::npos)
        {
            value = argument.substr(equalPosition + 1);
        }
        else if (index + 1 < arguments.size() && !arguments[index + 1].starts_with("--"))
        {
            value = arguments[++index];
        }
        else
        {
            result.success = false;
            result.error = argumentError("missing value for --" + name);
            return result;
        }

        if (value.empty())
        {
            result.success = false;
            result.error = argumentError("missing value for --" + name);
            return result;
        }
        setValue(result.configuration, name, value);
    }
    return result;
}

bool ArgsParser::isSupportedName(const std::string& name)
{
    return name == "vfs" || name == "log" || name == "script" || name == "config";
}

void ArgsParser::setValue(Configuration& configuration, const std::string& name,
    const std::string& value)
{
    if (name == "vfs")
    {
        configuration.vfsPath = value;
    }
    else if (name == "log")
    {
        configuration.logPath = value;
    }
    else if (name == "script")
    {
        configuration.scriptPath = value;
    }
    else
    {
        configuration.configPath = value;
    }
}
