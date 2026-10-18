#pragma once

#include "Command.hpp"

class LsCommand final : public Command
{
public:
    std::string_view name() const override;
    CommandResult execute(const std::vector<std::string>& arguments) const override;
};
