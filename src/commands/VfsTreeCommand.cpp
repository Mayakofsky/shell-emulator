#include "VfsTreeCommand.hpp"

#include <string>

std::string_view VfsTreeCommand::name() const
{
    return "vfs-tree";
}

CommandResult VfsTreeCommand::execute(CommandContext& context,
    const std::vector<std::string>& arguments) const
{
    if (arguments.size() > 1)
    {
        return CommandResult{{}, "vfs-tree: too many arguments", false, 0};
    }

    const std::string path = arguments.empty() ? context.currentDirectory
        : arguments.front();
    std::string error;
    const std::vector<std::string> paths = context.vfs.listPaths(path, error);
    if (!error.empty())
    {
        return CommandResult{{}, "vfs-tree: no such path: " + path, false, 0};
    }

    std::string output;
    for (const std::string& entry : paths)
    {
        output += entry;
        output.push_back('\n');
    }
    return CommandResult{output, {}, false, 0};
}
