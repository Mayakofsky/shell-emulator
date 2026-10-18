#include "Lexer.hpp"

#include "Environment.hpp"

#include <utility>

namespace
{
bool isSeparator(char character)
{
    return character == ' ' || character == '\t';
}

bool isVariableStart(char character)
{
    return (character >= 'A' && character <= 'Z') ||
        (character >= 'a' && character <= 'z') || character == '_';
}

bool isVariablePart(char character)
{
    return isVariableStart(character) || (character >= '0' && character <= '9');
}

class Scanner
{
public:
    explicit Scanner(std::string_view input)
        : input_(input)
    {
    }

    LexerResult scan()
    {
        while (position_ < input_.size() && result_.success)
        {
            if (isSeparator(input_[position_]))
            {
                ++position_;
            }
            else
            {
                scanWord();
            }
        }
        return result_;
    }

private:
    void scanWord()
    {
        std::string value;
        bool active = false;
        char quote = '\0';

        while (position_ < input_.size() &&
            (quote != '\0' || !isSeparator(input_[position_])))
        {
            scanCharacter(quote, value, active);
            if (!result_.success)
            {
                return;
            }
        }

        if (quote != '\0')
        {
            result_.success = false;
            result_.error = "syntax error: unterminated quote";
            return;
        }

        if (active)
        {
            result_.tokens.push_back(Token{std::move(value)});
        }
    }

    void scanCharacter(char& quote, std::string& value, bool& active)
    {
        const char character = input_[position_];
        if (quote == '\'')
        {
            scanSingleQuoted(character, quote, value);
            active = true;
        }
        else if (quote == '"')
        {
            scanDoubleQuoted(character, quote, value);
            active = true;
        }
        else
        {
            scanUnquoted(character, quote, value, active);
        }
    }

    void scanSingleQuoted(char character, char& quote, std::string& value)
    {
        if (character == '\'')
        {
            quote = '\0';
        }
        else
        {
            value.push_back(character);
        }
        ++position_;
    }

    void scanDoubleQuoted(char character, char& quote, std::string& value)
    {
        if (character == '"')
        {
            quote = '\0';
            ++position_;
        }
        else if (character == '\\')
        {
            scanEscape(value);
        }
        else if (character == '$')
        {
            expandVariable(value);
        }
        else
        {
            value.push_back(character);
            ++position_;
        }
    }

    void scanUnquoted(char character, char& quote, std::string& value, bool& active)
    {
        if (character == '\'' || character == '"')
        {
            quote = character;
            ++position_;
            active = true;
        }
        else if (character == '\\')
        {
            scanEscape(value);
            active = true;
        }
        else if (character == '$')
        {
            expandVariable(value);
            active = true;
        }
        else
        {
            value.push_back(character);
            ++position_;
            active = true;
        }
    }

    void scanEscape(std::string& value)
    {
        if (position_ + 1 < input_.size() && isEscapable(input_[position_ + 1]))
        {
            value.push_back(input_[position_ + 1]);
            position_ += 2;
            return;
        }
        value.push_back('\\');
        ++position_;
    }

    bool isEscapable(char character) const
    {
        return character == '$' || character == '\\' || character == '"' ||
            character == '\'';
    }

    void expandVariable(std::string& value)
    {
        ++position_;
        if (position_ >= input_.size())
        {
            value.push_back('$');
            return;
        }

        if (input_[position_] == '{')
        {
            expandBracedVariable(value);
        }
        else if (isVariableStart(input_[position_]))
        {
            expandNamedVariable(value);
        }
        else
        {
            value.push_back('$');
        }
    }

    void expandBracedVariable(std::string& value)
    {
        const std::size_t nameStart = position_ + 1;
        const std::size_t closingBrace = input_.find('}', nameStart);
        if (closingBrace == std::string_view::npos)
        {
            result_.success = false;
            result_.error = "syntax error: invalid ${...} expansion";
            return;
        }

        appendVariable(value, input_.substr(nameStart, closingBrace - nameStart));
        position_ = closingBrace + 1;
    }

    void expandNamedVariable(std::string& value)
    {
        const std::size_t nameStart = position_;
        while (position_ < input_.size() && isVariablePart(input_[position_]))
        {
            ++position_;
        }
        appendVariable(value, input_.substr(nameStart, position_ - nameStart));
    }

    void appendVariable(std::string& value, std::string_view name)
    {
        const auto variable = Environment::getVariable(name);
        if (variable)
        {
            value += *variable;
        }
    }

    std::string_view input_;
    std::size_t position_ = 0;
    LexerResult result_;
};
}

LexerResult Lexer::tokenize(std::string_view input) const
{
    return Scanner(input).scan();
}
