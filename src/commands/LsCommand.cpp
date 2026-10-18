#include "LsCommand.hpp"

#include <sstream>

std::string_view LsCommand::name() const
{
    return "ls";
}

CommandResult LsCommand::execute(const std::vector<std::string>& arguments) const
{
    std::ostringstream output;
    output << name() << " called with args: [";
    for (std::size_t index = 0; index < arguments.size(); ++index)
    {
        if (index > 0)
        {
            output << ", ";
        }
        output << arguments[index];
    }
    output << "]\n";
    return CommandResult{output.str(), {}, false, 0};
}
