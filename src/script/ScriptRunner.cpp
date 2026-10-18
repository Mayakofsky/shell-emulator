#include "ScriptRunner.hpp"

#include "shell/Shell.hpp"

#include <fstream>

int ScriptRunner::run(const std::string& path, Shell& shell, std::ostream& output,
    std::ostream& error) const
{
    std::ifstream input(path);
    if (!input)
    {
        error << "shell: script error: cannot read file: " << path << '\n';
        return 1;
    }

    std::string line;
    while (readLine(input, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        if (line.find_first_not_of(" \t\r\n") == std::string::npos)
        {
            continue;
        }
        output << shell.prompt() << line << '\n' << std::flush;
        const int result = shell.executeLine(line, output, error);
        if (shell.lastCommandExited())
        {
            return result;
        }
    }
    if (input.bad())
    {
        error << "shell: script error: read failure: " << path << '\n';
        return 1;
    }
    return 0;
}

bool ScriptRunner::readLine(std::istream& input, std::string& line)
{
    return static_cast<bool>(std::getline(input, line));
}
