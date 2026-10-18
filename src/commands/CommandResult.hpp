#pragma once

#include <string>

struct CommandResult
{
    std::string output;
    std::string error;
    bool shouldExit = false;
    int exitCode = 0;
};
