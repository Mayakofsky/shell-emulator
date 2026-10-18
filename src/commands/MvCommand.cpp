#include "MvCommand.hpp"

#include <string>
#include <vector>

std::string_view MvCommand::name() const
{
    return "mv";
}

CommandResult MvCommand::execute(CommandContext& context,
    const std::vector<std::string>& arguments) const
{
    if (arguments.empty())
    {
        return CommandResult{{}, "mv: missing file operand", false, 0};
    }
    if (arguments.size() == 1)
    {
        return CommandResult{{},
            "mv: missing destination file operand after '" + arguments.front() + "'",
            false, 0};
    }
    if (arguments.size() > 2)
    {
        return CommandResult{{}, "mv: too many arguments", false, 0};
    }

    std::string error;
    if (!context.vfs.moveEntry(context.currentDirectory, arguments[0],
        arguments[1], error))
    {
        return CommandResult{{}, "mv: " + error, false, 0};
    }
    return CommandResult{{}, {}, false, 0};
}
