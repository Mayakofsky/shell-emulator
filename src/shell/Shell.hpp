#pragma once

#include "Parser.hpp"
#include "PromptBuilder.hpp"
#include "commands/CommandRegistry.hpp"

#include <csignal>
#include <iosfwd>

class Shell
{
public:
    static void installSignalHandler();
    int run();
    int run(std::istream& input, std::ostream& output, std::ostream& error);

private:
    static void handleInterrupt(int signalNumber);
    std::string buildPrompt() const;

    static volatile std::sig_atomic_t interrupted_;
    Parser parser_;
    PromptBuilder promptBuilder_;
    CommandRegistry commandRegistry_;
};
