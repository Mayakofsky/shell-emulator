#include "CdCommand.hpp"

#include <sstream>

std::string_view CdCommand::name() const
{
    return "cd";
}

CommandResult CdCommand::execute(const std::vector<std::string>& arguments) const
{
    if (arguments.size() > 1)
    {
        return CommandResult{{}, "cd: too many arguments", false, 0};
    }

    std::ostringstream output;
    output << name() << " called with args: [";
    if (!arguments.empty())
    {
        output << arguments.front();
    }
    output << "]\n";
    return CommandResult{output.str(), {}, false, 0};
}
