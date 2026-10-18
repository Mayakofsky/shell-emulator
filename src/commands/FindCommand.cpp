#include "FindCommand.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace
{
struct Options
{
    std::string path;
    bool hasPath = false;
    std::string namePattern;
    bool hasName = false;
    bool typeDirectory = false;
    bool hasType = false;
    bool typeFile = false;
};

bool globMatch(const std::string& pattern, const std::string& name)
{
    std::size_t patternIndex = 0;
    std::size_t nameIndex = 0;
    std::size_t starPattern = std::string::npos;
    std::size_t starName = 0;
    while (nameIndex < name.size())
    {
        const bool single = patternIndex < pattern.size() &&
            (pattern[patternIndex] == '?' || pattern[patternIndex] == name[nameIndex]);
        if (single)
        {
            ++patternIndex;
            ++nameIndex;
        }
        else if (patternIndex < pattern.size() && pattern[patternIndex] == '*')
        {
            starPattern = patternIndex;
            starName = nameIndex;
            ++patternIndex;
        }
        else if (starPattern != std::string::npos)
        {
            patternIndex = starPattern + 1;
            ++starName;
            nameIndex = starName;
        }
        else
        {
            return false;
        }
    }
    while (patternIndex < pattern.size() && pattern[patternIndex] == '*')
    {
        ++patternIndex;
    }
    return patternIndex == pattern.size();
}

std::string join(const std::string& base, const std::string& name)
{
    if (base == "/")
    {
        return "/" + name;
    }
    return base + "/" + name;
}

void collect(const VfsNode& node, const std::string& prefix,
    const Options& options, std::vector<std::string>& results)
{
    for (const std::unique_ptr<VfsNode>& child : node.children)
    {
        const std::string childPath = join(prefix, child->name);
        const bool typeMatches = !options.hasType ||
            (child->directory ? options.typeDirectory : options.typeFile);
        const bool nameMatches = !options.hasName ||
            globMatch(options.namePattern, child->name);
        if (typeMatches && nameMatches)
        {
            results.push_back(child->directory ? childPath + "/" : childPath);
        }
        if (child->directory)
        {
            collect(*child, childPath, options, results);
        }
    }
}
}

std::string_view FindCommand::name() const
{
    return "find";
}

CommandResult FindCommand::execute(CommandContext& context,
    const std::vector<std::string>& arguments) const
{
    Options options;
    for (std::size_t index = 0; index < arguments.size(); ++index)
    {
        const std::string& argument = arguments[index];
        if (argument == "-name")
        {
            if (index + 1 >= arguments.size())
            {
                return CommandResult{{}, "find: missing argument for -name", false, 0};
            }
            options.namePattern = arguments[++index];
            options.hasName = true;
        }
        else if (argument == "-type")
        {
            if (index + 1 >= arguments.size())
            {
                return CommandResult{{}, "find: missing argument for -type", false, 0};
            }
            const std::string& value = arguments[++index];
            if (value == "f")
            {
                options.typeFile = true;
                options.hasType = true;
            }
            else if (value == "d")
            {
                options.typeDirectory = true;
                options.hasType = true;
            }
            else
            {
                return CommandResult{{}, "find: invalid argument for -type: " + value,
                    false, 0};
            }
        }
        else if (argument.size() > 1 && argument.front() == '-')
        {
            return CommandResult{{}, "find: unknown predicate: " + argument, false, 0};
        }
        else if (!options.hasPath)
        {
            options.path = argument;
            options.hasPath = true;
        }
        else
        {
            return CommandResult{{}, "find: unexpected argument: " + argument, false, 0};
        }
    }

    const std::string path = options.hasPath ? options.path : context.currentDirectory;
    std::string canonical;
    const VfsNode* start = context.vfs.resolveFrom(
        context.currentDirectory, path, canonical);
    if (start == nullptr)
    {
        return CommandResult{{}, "find: no such path: " + path, false, 0};
    }

    std::vector<std::string> results;
    collect(*start, canonical, options, results);
    std::sort(results.begin(), results.end());

    std::string output;
    for (const std::string& entry : results)
    {
        output += entry;
        output.push_back('\n');
    }
    return CommandResult{output, {}, false, 0};
}
