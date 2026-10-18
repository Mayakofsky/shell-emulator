#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

class Environment
{
public:
    static std::optional<std::string> getVariable(std::string_view name);
    static std::string username();
    static std::string hostname();
    static std::filesystem::path homeDirectory();
    static std::filesystem::path currentDirectory();
};
