#include "CdCommand.hpp"

#include <string>
#include <vector>

std::string_view CdCommand::name() const
{
    return "cd";
}

CommandResult CdCommand::execute(CommandContext& context,
    const std::vector<std::string>& arguments) const
{
    if (arguments.size() > 1)
    {
        return CommandResult{{}, "cd: too many arguments", false, 0};
    }

    const std::string target = arguments.empty() ? "/" : arguments.front();
    std::string canonical;
    const VfsNode* node = context.vfs.resolveFrom(
        context.currentDirectory, target, canonical);
    if (node == nullptr)
    {
        return CommandResult{{}, "cd: no such directory: " + target, false, 0};
    }
    if (!node->directory)
    {
        return CommandResult{{}, "cd: not a directory: " + target, false, 0};
    }

    context.currentDirectory = canonical;
    return CommandResult{{}, {}, false, 0};
}
