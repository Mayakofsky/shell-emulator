#pragma once

#include <istream>
#include <ostream>
#include <string>

class Shell;

class ScriptRunner
{
public:
    int run(const std::string& path, Shell& shell, std::ostream& output,
        std::ostream& error) const;

private:
    static bool readLine(std::istream& input, std::string& line);
};
