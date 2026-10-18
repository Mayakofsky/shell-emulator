#include "WcCommand.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace
{
struct Counts
{
    std::size_t lines = 0;
    std::size_t words = 0;
    std::size_t bytes = 0;
};

struct Options
{
    bool lines = false;
    bool words = false;
    bool bytes = false;
    bool any = false;
};

bool isWhitespace(std::uint8_t byte)
{
    return byte == ' ' || byte == '\t' || byte == '\n' ||
        byte == '\r' || byte == '\v' || byte == '\f';
}

Counts countContent(const std::vector<std::uint8_t>& data)
{
    Counts counts;
    counts.bytes = data.size();
    bool inWord = false;
    for (const std::uint8_t byte : data)
    {
        if (byte == '\n')
        {
            ++counts.lines;
        }
        if (isWhitespace(byte))
        {
            inWord = false;
        }
        else if (!inWord)
        {
            inWord = true;
            ++counts.words;
        }
    }
    return counts;
}

bool isOption(const std::string& argument)
{
    return argument.size() > 1 && argument.front() == '-';
}

bool addOptions(const std::string& argument, Options& options, std::string& error)
{
    for (std::size_t index = 1; index < argument.size(); ++index)
    {
        switch (argument[index])
        {
        case 'l':
            options.lines = true;
            options.any = true;
            break;
        case 'w':
            options.words = true;
            options.any = true;
            break;
        case 'c':
            options.bytes = true;
            options.any = true;
            break;
        default:
            error = std::string("wc: invalid option: -") + argument[index];
            return false;
        }
    }
    return true;
}

std::string formatCounts(const Counts& counts, const Options& options)
{
    std::string result;
    bool first = true;
    const auto append = [&result, &first](std::size_t value)
    {
        if (!first)
        {
            result.push_back(' ');
        }
        first = false;
        result += std::to_string(value);
    };
    if (options.lines)
    {
        append(counts.lines);
    }
    if (options.words)
    {
        append(counts.words);
    }
    if (options.bytes)
    {
        append(counts.bytes);
    }
    return result;
}
}

std::string_view WcCommand::name() const
{
    return "wc";
}

CommandResult WcCommand::execute(CommandContext& context,
    const std::vector<std::string>& arguments) const
{
    Options options;
    std::vector<std::string> files;
    for (const std::string& argument : arguments)
    {
        if (isOption(argument))
        {
            std::string error;
            if (!addOptions(argument, options, error))
            {
                return CommandResult{{}, error, false, 0};
            }
            continue;
        }
        files.push_back(argument);
    }
    if (files.empty())
    {
        return CommandResult{{}, "wc: missing file operand", false, 0};
    }
    if (!options.any)
    {
        options.lines = true;
        options.words = true;
        options.bytes = true;
    }

    std::string output;
    Counts total;
    for (const std::string& file : files)
    {
        std::string canonical;
        const VfsNode* node = context.vfs.resolveFrom(
            context.currentDirectory, file, canonical);
        if (node == nullptr)
        {
            return CommandResult{{}, "wc: no such file: " + file, false, 0};
        }
        if (node->directory)
        {
            return CommandResult{{}, "wc: is a directory: " + file, false, 0};
        }

        const Counts counts = countContent(node->data);
        total.lines += counts.lines;
        total.words += counts.words;
        total.bytes += counts.bytes;
        output += formatCounts(counts, options);
        output += " " + file + "\n";
    }
    if (files.size() > 1)
    {
        output += formatCounts(total, options) + " total\n";
    }
    return CommandResult{output, {}, false, 0};
}
