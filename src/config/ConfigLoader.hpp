#pragma once

#include "ArgsParser.hpp"
#include "Configuration.hpp"

#include <string>
#include <vector>

struct ConfigLoadResult
{
    Configuration configuration;
    std::string error;
    bool success = true;
};

class ConfigLoader
{
public:
    ConfigLoadResult load(const std::vector<std::string>& arguments) const;

private:
    static Configuration merge(const Configuration& fileConfiguration,
        const Configuration& cliConfiguration);
};
