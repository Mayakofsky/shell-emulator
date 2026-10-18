#pragma once

#include "Lexer.hpp"

#include <string>
#include <string_view>
#include <vector>

struct ParseResult
{
    bool hasCommand = false;
    std::string command;
    std::vector<std::string> arguments;
    std::string error;
    bool success = true;
};

class Parser
{
public:
    ParseResult parse(std::string_view input) const;

private:
    Lexer lexer_;
};
