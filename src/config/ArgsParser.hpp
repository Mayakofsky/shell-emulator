#pragma once

#include "Configuration.hpp"

#include <string>
#include <vector>

struct ArgsParseResult
{
    Configuration configuration;
    std::string error;
    bool success = true;
};

class ArgsParser
{
public:
    ArgsParseResult parse(const std::vector<std::string>& arguments) const;

private:
    static bool isSupportedName(const std::string& name);
    static void setValue(Configuration& configuration, const std::string& name,
        const std::string& value);
};
