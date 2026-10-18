#include "RmCommand.hpp"

#include <cstddef>
#include <string>
#include <vector>

std::string_view RmCommand::name() const
{
    return "rm";
}

CommandResult RmCommand::execute(CommandContext& context,
    const std::vector<std::string>& arguments) const
{
    bool recursive = false;
    bool force = false;
    std::vector<std::string> paths;
    for (const std::string& argument : arguments)
    {
        if (argument.size() > 1 && argument.front() == '-')
        {
            for (std::size_t index = 1; index < argument.size(); ++index)
            {
                switch (argument[index])
                {
                case 'r':
                case 'R':
                    recursive = true;
                    break;
                case 'f':
                    force = true;
                    break;
                default:
                    return CommandResult{{},
                        std::string("rm: invalid option: -") + argument[index], false, 0};
                }
            }
            continue;
        }
        paths.push_back(argument);
    }
    if (paths.empty())
    {
        return CommandResult{{}, "rm: missing operand", false, 0};
    }

    for (const std::string& path : paths)
    {
        std::string error;
        if (context.vfs.removeEntry(context.currentDirectory, path, recursive, error))
        {
            continue;
        }
        if (force && error.rfind("no such file:", 0) == 0)
        {
            continue;
        }
        return CommandResult{{}, "rm: " + error, false, 0};
    }
    return CommandResult{{}, {}, false, 0};
}
