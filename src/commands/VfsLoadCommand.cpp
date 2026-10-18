#include "VfsLoadCommand.hpp"

#include <string>

std::string_view VfsLoadCommand::name() const
{
    return "vfs-load";
}

CommandResult VfsLoadCommand::execute(CommandContext& context,
    const std::vector<std::string>& arguments) const
{
    if (arguments.empty())
    {
        return CommandResult{{}, "vfs-load: missing archive path", false, 0};
    }
    if (arguments.size() > 1)
    {
        return CommandResult{{}, "vfs-load: too many arguments", false, 0};
    }

    const std::string& path = arguments.front();
    Vfs loaded;
    std::string error;
    if (!loaded.loadFromArchive(path, error))
    {
        return CommandResult{{}, "shell: vfs error: " + error, false, 0};
    }

    const std::size_t entries = loaded.entryCount();
    context.vfs = std::move(loaded);
    return CommandResult{
        "vfs: loaded " + std::to_string(entries) +
            " entries from " + path + "\n", {}, false, 0};
}
