#pragma once

#include "CommandResult.hpp"

#include <string_view>
#include <vector>

class Command
{
public:
    virtual ~Command() = default;
    virtual std::string_view name() const = 0;
    virtual CommandResult execute(const std::vector<std::string>& arguments) const = 0;
};
