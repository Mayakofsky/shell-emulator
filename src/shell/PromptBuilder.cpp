#include "PromptBuilder.hpp"

#include <algorithm>
#include <cctype>

std::string PromptBuilder::build(
    std::string_view username,
    std::string_view hostname,
    const std::filesystem::path& currentDirectory,
    const std::filesystem::path& homeDirectory) const
{
    std::string directory = normalizePath(currentDirectory);
    const std::string home = normalizePath(homeDirectory);
    if (!home.empty() && pathsEqual(directory, home))
    {
        directory = "~";
    }

    return std::string(username) + "@" + std::string(hostname) + ":" + directory + "$ ";
}

std::string PromptBuilder::normalizePath(const std::filesystem::path& path)
{
    std::string result = path.lexically_normal().string();
    std::replace(result.begin(), result.end(), '\\', '/');
    while (result.size() > 1 && result.back() == '/' &&
        !(result.size() == 3 && result[1] == ':'))
    {
        result.pop_back();
    }
    return result;
}

bool PromptBuilder::pathsEqual(std::string left, std::string right)
{
#ifdef _WIN32
    const auto lowerCharacter = [](unsigned char character)
    {
        return static_cast<char>(std::tolower(character));
    };
    std::transform(left.begin(), left.end(), left.begin(), lowerCharacter);
    std::transform(right.begin(), right.end(), right.begin(), lowerCharacter);
#endif
    return left == right;
}
