#pragma once

#include "Parser.hpp"
#include "PromptBuilder.hpp"
#include "commands/CommandRegistry.hpp"
#include "vfs/Vfs.hpp"

#include <csignal>
#include <iosfwd>
#include <optional>
#include <string>

class JsonLogger;

class Shell
{
public:
    explicit Shell(JsonLogger* logger = nullptr,
        std::optional<std::string> startupScript = std::nullopt);
    static void installSignalHandler();
    int run();
    int run(std::istream& input, std::ostream& output, std::ostream& error);
    int executeLine(const std::string& line, std::ostream& output, std::ostream& error);
    std::string prompt() const;
    bool lastCommandExited() const;
    bool loadVfs(const std::string& path, std::string& error);
    std::size_t vfsEntryCount() const;
private:
    static void handleInterrupt(int signalNumber);
    std::string buildPrompt() const;
    static bool isBlank(const std::string& line);

    static volatile std::sig_atomic_t interrupted_;
    Parser parser_;
    PromptBuilder promptBuilder_;
    CommandRegistry commandRegistry_;
    JsonLogger* logger_ = nullptr;
    std::optional<std::string> startupScript_;
    bool lastCommandExited_ = false;
    Vfs vfs_;
    std::string currentDirectory_ = "/";
};
