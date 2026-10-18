#include "ExitCommand.hpp"

#include <charconv>
#include <string_view>

namespace
{
bool parseExitCode(std::string_view value, int& code)
{
    if (value.empty())
    {
        return false;
    }
    if (value.front() == '+')
    {
        value.remove_prefix(1);
        if (value.empty())
        {
            return false;
        }
    }

    const char* const end = value.data() + value.size();
    const auto parsed = std::from_chars(value.data(), end, code);
    return parsed.ec == std::errc{} && parsed.ptr == end;
}
}

std::string_view ExitCommand::name() const
{
    return "exit";
}

CommandResult ExitCommand::execute([[maybe_unused]] CommandContext& context,
    const std::vector<std::string>& arguments) const
{
    if (arguments.size() > 1)
    {
        return CommandResult{{}, "exit: too many arguments", false, 0};
    }
    if (arguments.empty())
    {
        return CommandResult{{}, {}, true, 0};
    }

    int exitCode = 0;
    if (!parseExitCode(arguments.front(), exitCode))
    {
        return CommandResult{{}, "exit: numeric argument required", false, 0};
    }
    return CommandResult{{}, {}, true, exitCode};
}
