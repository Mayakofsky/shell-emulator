#pragma once

#include "Configuration.hpp"

#include <string>

struct ConfigFileResult
{
    Configuration configuration;
    std::string error;
    bool success = true;
};

class ConfigFileReader
{
public:
    ConfigFileResult read(const std::string& path) const;
};
