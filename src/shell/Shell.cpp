#include "Shell.hpp"

#include "Environment.hpp"
#include "log/JsonLogger.hpp"
#include "script/ScriptRunner.hpp"

#include <iostream>
#include <string>
#include <utility>

volatile std::sig_atomic_t Shell::interrupted_ = 0;

Shell::Shell(JsonLogger* logger, std::optional<std::string> startupScript)
    : logger_(logger), startupScript_(std::move(startupScript))
{
}

void Shell::installSignalHandler()
{
    interrupted_ = 0;
    std::signal(SIGINT, &Shell::handleInterrupt);
}

int Shell::run()
{
    if (startupScript_)
    {
        const int scriptCode = ScriptRunner{}.run(
            *startupScript_, *this, std::cout, std::cerr);
        if (lastCommandExited_ || scriptCode != 0)
        {
            return scriptCode;
        }
    }
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

        const int result = executeLine(line, output, error);
        if (lastCommandExited_)
        {
            return result;
        }
    }
    return 0;
}

int Shell::executeLine(const std::string& line, std::ostream& output, std::ostream& error)
{
    lastCommandExited_ = false;
    if (isBlank(line))
    {
        return 0;
    }

    const ParseResult parsed = parser_.parse(line);
    if (!parsed.success)
    {
        if (logger_ != nullptr)
        {
            logger_->log(std::nullopt, {}, "syntax-error");
        }
        error << parsed.error << '\n';
        return 0;
    }
    if (!parsed.hasCommand)
    {
        if (logger_ != nullptr)
        {
            logger_->log(std::string(), {}, "error");
        }
        return 0;
    }

    const CommandResult result = commandRegistry_.execute(parsed.command, parsed.arguments);
    const std::string status = result.shouldExit ? "exit" :
        (result.error.empty() ? (commandRegistry_.contains(parsed.command) ? "ok" : "unknown") :
        (commandRegistry_.contains(parsed.command) ? "error" : "unknown"));
    if (logger_ != nullptr)
    {
        logger_->log(parsed.command, parsed.arguments, status);
    }
    if (!result.output.empty())
    {
        output << result.output;
    }
    if (!result.error.empty())
    {
        error << result.error << '\n';
    }
    lastCommandExited_ = result.shouldExit;
    return result.exitCode;
}

std::string Shell::prompt() const
{
    return buildPrompt();
}

bool Shell::lastCommandExited() const
{
    return lastCommandExited_;
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

bool Shell::isBlank(const std::string& line)
{
    return line.find_first_not_of(" \t\r\n") == std::string::npos;
}
