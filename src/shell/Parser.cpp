#include "Parser.hpp"

ParseResult Parser::parse(std::string_view input) const
{
    const LexerResult lexed = lexer_.tokenize(input);
    if (!lexed.success)
    {
        return ParseResult{false, {}, {}, "shell: " + lexed.error, false};
    }

    if (lexed.tokens.empty() || lexed.tokens.front().value.empty())
    {
        return {};
    }

    ParseResult result;
    result.hasCommand = true;
    result.command = lexed.tokens.front().value;
    result.arguments.reserve(lexed.tokens.size() - 1);
    for (std::size_t index = 1; index < lexed.tokens.size(); ++index)
    {
        result.arguments.push_back(lexed.tokens[index].value);
    }
    return result;
}
