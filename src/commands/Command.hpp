#pragma once

#include "CommandContext.hpp"
#include "CommandResult.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

class Command
{
public:
    virtual ~Command() = default;
    virtual std::string_view name() const = 0;
    virtual CommandResult execute(CommandContext& context,
        const std::vector<std::string>& arguments) const = 0;
};
