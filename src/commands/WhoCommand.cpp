#include "WhoCommand.hpp"

#include "shell/Environment.hpp"

#include <string>
#include <vector>

std::string_view WhoCommand::name() const
{
    return "who";
}

CommandResult WhoCommand::execute(CommandContext& context,
    const std::vector<std::string>& arguments) const
{
    (void)context;
    if (!arguments.empty())
    {
        return CommandResult{{}, "who: too many arguments", false, 0};
    }
    return CommandResult{Environment::username() + " " + Environment::hostname() + "\n",
        {}, false, 0};
}
