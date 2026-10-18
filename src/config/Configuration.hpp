#pragma once

#include <optional>
#include <string>

struct Configuration
{
    std::optional<std::string> configPath;
    std::optional<std::string> vfsPath;
    std::optional<std::string> logPath;
    std::optional<std::string> scriptPath;
};
