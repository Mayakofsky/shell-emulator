#pragma once

#include "Token.hpp"

#include <string>
#include <string_view>
#include <vector>

struct LexerResult
{
    std::vector<Token> tokens;
    std::string error;
    bool success = true;
};

class Lexer
{
public:
    LexerResult tokenize(std::string_view input) const;
};
