#include "VfsReadCommand.hpp"

#include "vfs/Base64.hpp"

#include <string>

std::string_view VfsReadCommand::name() const
{
    return "vfs-read";
}

CommandResult VfsReadCommand::execute(CommandContext& context,
    const std::vector<std::string>& arguments) const
{
    if (arguments.empty())
    {
        return CommandResult{{}, "vfs-read: missing path", false, 0};
    }
    if (arguments.size() > 1)
    {
        return CommandResult{{}, "vfs-read: too many arguments", false, 0};
    }

    const std::string& path = arguments.front();
    const VfsNode* node = context.vfs.resolve(path);
    if (node == nullptr)
    {
        return CommandResult{{}, "vfs-read: no such file: " + path, false, 0};
    }
    if (node->directory)
    {
        return CommandResult{{}, "vfs-read: is a directory: " + path, false, 0};
    }

    std::string content;
    if (Base64::isBinary(node->data))
    {
        content = "base64:" + Base64::encode(node->data);
    }
    else
    {
        content.assign(node->data.begin(), node->data.end());
    }
    content.push_back('\n');
    return CommandResult{content, {}, false, 0};
}
