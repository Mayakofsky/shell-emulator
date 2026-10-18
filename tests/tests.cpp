#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "commands/CommandRegistry.hpp"
#include "commands/ExitCommand.hpp"
#include "config/ArgsParser.hpp"
#include "config/ConfigLoader.hpp"
#include "log/JsonLogger.hpp"
#include "script/ScriptRunner.hpp"
#include "shell/Lexer.hpp"
#include "shell/Parser.hpp"
#include "shell/PromptBuilder.hpp"
#include "shell/Shell.hpp"

namespace
{
int failures = 0;

void check(bool condition, const std::string& name)
{
    if (condition)
    {
        std::cout << "ok   " << name << '\n';
    }
    else
    {
        std::cout << "FAIL " << name << '\n';
        ++failures;
    }
}

std::filesystem::path writeTemp(const std::string& name, const std::string& content)
{
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / name;
    std::ofstream file(path, std::ios::trunc);
    file << content;
    return path;
}
}

int main()
{
    const LexerResult tokens = Lexer{}.tokenize("  ls   first\tsecond ");
    check(tokens.success, "lexer splits arguments");
    check(tokens.tokens.size() == 3 && tokens.tokens[1].value == "first",
        "lexer keeps argument order");

    const LexerResult quotes = Lexer{}.tokenize("cmd 'one two' \"three four\"");
    check(quotes.success && quotes.tokens.size() == 3, "lexer understands quotes");
    check(quotes.tokens[1].value == "one two" && quotes.tokens[2].value == "three four",
        "lexer removes quotes");

    const ParseResult parsed = Parser{}.parse("ls\t-l 'folder name'");
    check(parsed.success && parsed.command == "ls", "parser reads the command");
    check(parsed.arguments == std::vector<std::string>({"-l", "folder name"}),
        "parser reads the arguments");

    const ParseResult broken = Parser{}.parse("ls \"unfinished");
    check(!broken.success && !broken.error.empty(), "parser reports syntax errors");

    const std::string prompt = PromptBuilder{}.build("alice", "host",
        std::filesystem::path("/home/alice"), std::filesystem::path("/home/alice"));
    check(prompt == "alice@host:~$ ", "prompt hides the home directory");

    const CommandResult exitCode = ExitCommand{}.execute({"-3"});
    check(exitCode.shouldExit && exitCode.exitCode == -3, "exit keeps the code");

    const CommandResult unknown = CommandRegistry{}.execute("missing", {});
    check(unknown.error == "shell: unknown command: missing", "unknown command is reported");

    const ArgsParseResult args = ArgsParser{}.parse(
        {"--vfs", "vfs-a", "--log=log-a", "--script", "script-a"});
    check(args.success && args.configuration.vfsPath == "vfs-a", "argument parser reads vfs");
    check(args.configuration.logPath == "log-a" &&
        args.configuration.scriptPath == "script-a", "argument parser reads log and script");
    const ArgsParseResult badArgs = ArgsParser{}.parse({"--bad", "x"});
    check(!badArgs.success && !badArgs.error.empty(), "argument parser reports bad parameters");

    const std::filesystem::path configPath =
        writeTemp("shell_emulator_test_config.xml",
            "<config><vfs>file-vfs</vfs><log>file-log</log></config>");
    const ConfigLoadResult loaded = ConfigLoader{}.load(
        {"--config", configPath.string(), "--vfs=cli-vfs"});
    std::filesystem::remove(configPath);
    check(loaded.success && loaded.configuration.vfsPath == "cli-vfs",
        "command line overrides the config file");
    check(!loaded.configuration.scriptPath, "missing values stay empty");

    check(JsonLogger::escape("a\"b\\c\nd") == "a\\\"b\\\\c\\u000ad",
        "logger escapes json characters");

    const std::filesystem::path scriptPath =
        writeTemp("shell_emulator_test_script.txt", "ls\nmissing\nexit 4\n");
    std::ostringstream scriptOutput;
    std::ostringstream scriptError;
    Shell scriptShell;
    const int scriptCode = ScriptRunner{}.run(
        scriptPath.string(), scriptShell, scriptOutput, scriptError);
    std::filesystem::remove(scriptPath);
    check(scriptCode == 4, "startup script stops on exit");
    check(scriptError.str().find("unknown command") != std::string::npos,
        "startup script skips broken lines");

    std::istringstream input("ls\nmissing\nexit 0\n");
    std::ostringstream output;
    std::ostringstream error;
    Shell shell;
    shell.run(input, output, error);
    check(error.str().find("unknown command") != std::string::npos,
        "shell survives unknown commands");

    if (failures == 0)
    {
        std::cout << "all tests passed\n";
        return 0;
    }
    std::cout << failures << " test(s) failed\n";
    return 1;
}
