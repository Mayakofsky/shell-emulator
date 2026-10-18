#pragma once

#include <filesystem>
#include <string>
#include <string_view>

class PromptBuilder
{
public:
    std::string build(
        std::string_view username,
        std::string_view hostname,
        const std::filesystem::path& currentDirectory,
        const std::filesystem::path& homeDirectory) const;

private:
    static std::string normalizePath(const std::filesystem::path& path);
    static bool pathsEqual(std::string left, std::string right);
};
