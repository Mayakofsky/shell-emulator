#include "CommandRegistry.hpp"

#include "CdCommand.hpp"
#include "ExitCommand.hpp"
#include "FindCommand.hpp"
#include "LsCommand.hpp"
#include "VfsLoadCommand.hpp"
#include "VfsReadCommand.hpp"
#include "VfsTreeCommand.hpp"
#include "WcCommand.hpp"
#include "WhoCommand.hpp"

CommandRegistry::CommandRegistry()
{
    registerCommand(std::make_unique<LsCommand>());
    registerCommand(std::make_unique<CdCommand>());
    registerCommand(std::make_unique<WcCommand>());
    registerCommand(std::make_unique<FindCommand>());
    registerCommand(std::make_unique<WhoCommand>());
    registerCommand(std::make_unique<ExitCommand>());
    registerCommand(std::make_unique<VfsLoadCommand>());
    registerCommand(std::make_unique<VfsTreeCommand>());
    registerCommand(std::make_unique<VfsReadCommand>());
}

void CommandRegistry::registerCommand(std::unique_ptr<Command> command)
{
    if (command)
    {
        const std::string commandName(command->name());
        commands_.insert_or_assign(commandName, std::move(command));
    }
}

CommandResult CommandRegistry::execute(CommandContext& context,
    const std::string& name,
    const std::vector<std::string>& arguments) const
{
    const auto command = commands_.find(name);
    if (command == commands_.end())
    {
        return CommandResult{{}, "shell: unknown command: " + name, false, 0};
    }
    return command->second->execute(context, arguments);
}

bool CommandRegistry::contains(const std::string& name) const
{
    return commands_.contains(name);
}
