#include "CommandRegistry.hpp"

#include "CdCommand.hpp"
#include "ExitCommand.hpp"
#include "LsCommand.hpp"

CommandRegistry::CommandRegistry()
{
    registerCommand(std::make_unique<LsCommand>());
    registerCommand(std::make_unique<CdCommand>());
    registerCommand(std::make_unique<ExitCommand>());
}

void CommandRegistry::registerCommand(std::unique_ptr<Command> command)
{
    if (command)
    {
        const std::string commandName(command->name());
        commands_.insert_or_assign(commandName, std::move(command));
    }
}

CommandResult CommandRegistry::execute(
    const std::string& name,
    const std::vector<std::string>& arguments) const
{
    const auto command = commands_.find(name);
    if (command == commands_.end())
    {
        return CommandResult{{}, "shell: unknown command: " + name, false, 0};
    }
    return command->second->execute(arguments);
}

bool CommandRegistry::contains(const std::string& name) const
{
    return commands_.contains(name);
}
