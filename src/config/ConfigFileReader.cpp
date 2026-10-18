#include "ConfigFileReader.hpp"

#include <cctype>
#include <fstream>
#include <sstream>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace
{
struct XmlTag
{
    std::string name;
    bool closing = false;
    bool selfClosing = false;
};

std::string trim(std::string value)
{
    const auto notSpace = [](unsigned char character)
    {
        return std::isspace(character) == 0;
    };
    while (!value.empty() && !notSpace(static_cast<unsigned char>(value.front())))
    {
        value.erase(value.begin());
    }
    while (!value.empty() && !notSpace(static_cast<unsigned char>(value.back())))
    {
        value.pop_back();
    }
    return value;
}

bool parseTag(std::string_view text, XmlTag& tag)
{
    std::string content = trim(std::string(text));
    if (content.empty() || content.front() == '?' || content.front() == '!')
    {
        return true;
    }
    if (content.front() == '/')
    {
        tag.closing = true;
        content.erase(content.begin());
        content = trim(std::move(content));
    }
    if (!content.empty() && content.back() == '/')
    {
        tag.selfClosing = true;
        content.pop_back();
        content = trim(std::move(content));
    }
    if (content.find('<') != std::string::npos || content.find('>') != std::string::npos)
    {
        return false;
    }
    const std::size_t nameEnd = content.find_first_of(" \t\r\n");
    tag.name = content.substr(0, nameEnd);
    return !tag.name.empty();
}

class XmlReader
{
public:
    explicit XmlReader(std::string text)
        : text_(std::move(text))
    {
    }

    ConfigFileResult read()
    {
        ConfigFileResult result;
        std::vector<std::string> stack;
        std::size_t position = 0;
        bool rootSeen = false;
        while (position < text_.size())
        {
            if (text_[position] != '<')
            {
                const std::size_t end = text_.find('<', position);
                appendText(stack, text_.substr(position, end - position));
                position = end == std::string::npos ? text_.size() : end;
                continue;
            }
            const std::size_t end = text_.find('>', position + 1);
            if (end == std::string::npos)
            {
                return failure("unterminated tag");
            }
            XmlTag tag;
            if (!parseTag(text_.substr(position + 1, end - position - 1), tag))
            {
                return failure("invalid tag");
            }
            if (tag.name.empty())
            {
                position = end + 1;
                continue;
            }
            if (tag.closing)
            {
                if (stack.empty() || stack.back() != tag.name)
                {
                    return failure("mismatched closing tag");
                }
                stack.pop_back();
            }
            else
            {
                if (stack.empty())
                {
                    if (rootSeen || tag.name != "config")
                    {
                        return failure("root element must be config");
                    }
                    rootSeen = true;
                }
                stack.push_back(tag.name);
                if (tag.selfClosing)
                {
                    stack.pop_back();
                }
            }
            position = end + 1;
        }
        if (!rootSeen)
        {
            return failure("missing config root");
        }
        if (!stack.empty())
        {
            return failure("unclosed tag: " + stack.back());
        }
        applyValues();
        return result_;
    }

private:
    void appendText(const std::vector<std::string>& stack, std::string_view value)
    {
        if (stack.size() == 2 && stack.front() == "config" &&
            (stack.back() == "vfs" || stack.back() == "log" || stack.back() == "script"))
        {
            auto& target = values_[stack.back()];
            target += value;
        }
    }

    ConfigFileResult failure(const std::string& message)
    {
        result_.success = false;
        result_.error = message;
        return result_;
    }

    void applyValues()
    {
        assignValue("vfs", result_.configuration.vfsPath);
        assignValue("log", result_.configuration.logPath);
        assignValue("script", result_.configuration.scriptPath);
    }

    void assignValue(const std::string& name, std::optional<std::string>& target)
    {
        const auto value = values_.find(name);
        if (value != values_.end())
        {
            target = trim(value->second);
        }
    }

    std::string text_;
    ConfigFileResult result_;
    std::unordered_map<std::string, std::string> values_;
};
}

ConfigFileResult ConfigFileReader::read(const std::string& path) const
{
    std::ifstream input(path, std::ios::binary);
    if (!input)
    {
        return ConfigFileResult{{}, "cannot read file: " + path, false};
    }
    std::ostringstream content;
    content << input.rdbuf();
    XmlReader reader(content.str());
    ConfigFileResult result = reader.read();
    if (!result.success)
    {
        return result;
    }
    return result;
}
