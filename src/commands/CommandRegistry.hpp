#pragma once

#include "Command.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class CommandRegistry
{
public:
    CommandRegistry();
    void registerCommand(std::unique_ptr<Command> command);
    CommandResult execute(
        const std::string& name,
        const std::vector<std::string>& arguments) const;
    bool contains(const std::string& name) const;

private:
    std::unordered_map<std::string, std::unique_ptr<Command>> commands_;
};
