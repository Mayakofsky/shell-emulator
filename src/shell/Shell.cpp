#include "Shell.hpp"

#include "Environment.hpp"

#include <iostream>
#include <string>

volatile std::sig_atomic_t Shell::interrupted_ = 0;

void Shell::installSignalHandler()
{
    interrupted_ = 0;
    std::signal(SIGINT, &Shell::handleInterrupt);
}

int Shell::run()
{
    return run(std::cin, std::cout, std::cerr);
}

int Shell::run(std::istream& input, std::ostream& output, std::ostream& error)
{
    interrupted_ = 0;
    while (interrupted_ == 0)
    {
        output << buildPrompt() << std::flush;
        std::string line;
        if (!std::getline(input, line) || interrupted_ != 0)
        {
            break;
        }

        const ParseResult parsed = parser_.parse(line);
        if (!parsed.success)
        {
            error << parsed.error << '\n';
            continue;
        }
        if (!parsed.hasCommand)
        {
            continue;
        }

        const CommandResult result = commandRegistry_.execute(
            parsed.command, parsed.arguments);
        if (!result.output.empty())
        {
            output << result.output;
        }
        if (!result.error.empty())
        {
            error << result.error << '\n';
        }
        if (result.shouldExit)
        {
            return result.exitCode;
        }
    }
    return 0;
}

void Shell::handleInterrupt(int)
{
    interrupted_ = 1;
}

std::string Shell::buildPrompt() const
{
    return promptBuilder_.build(
        Environment::username(),
        Environment::hostname(),
        Environment::currentDirectory(),
        Environment::homeDirectory());
}
