#pragma once

#include "Command.hpp"

class WcCommand final : public Command
{
public:
    std::string_view name() const override;
    CommandResult execute(CommandContext& context,
        const std::vector<std::string>& arguments) const override;
};
