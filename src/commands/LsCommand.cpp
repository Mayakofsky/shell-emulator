#include "LsCommand.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

std::string_view LsCommand::name() const
{
    return "ls";
}

CommandResult LsCommand::execute(CommandContext& context,
    const std::vector<std::string>& arguments) const
{
    if (arguments.size() > 1)
    {
        return CommandResult{{}, "ls: too many arguments", false, 0};
    }

    const std::string path = arguments.empty() ? context.currentDirectory
        : arguments.front();
    std::string canonical;
    const VfsNode* node = context.vfs.resolveFrom(
        context.currentDirectory, path, canonical);
    if (node == nullptr)
    {
        return CommandResult{{}, "ls: no such path: " + path, false, 0};
    }
    if (!node->directory)
    {
        return CommandResult{{}, "ls: not a directory: " + path, false, 0};
    }

    std::vector<std::string> names;
    names.reserve(node->children.size());
    for (const std::unique_ptr<VfsNode>& child : node->children)
    {
        names.push_back(child->name);
    }
    std::sort(names.begin(), names.end());

    std::string output;
    for (const std::string& entry : names)
    {
        output += entry;
        output.push_back('\n');
    }
    return CommandResult{output, {}, false, 0};
}
