#pragma once

#include "Command.hpp"

class MvCommand final : public Command
{
public:
    std::string_view name() const override;
    CommandResult execute(CommandContext& context,
        const std::vector<std::string>& arguments) const override;
};
