#pragma once

#include "vfs/Vfs.hpp"

#include <string>

struct CommandContext
{
    Vfs& vfs;
    std::string& currentDirectory;
};
