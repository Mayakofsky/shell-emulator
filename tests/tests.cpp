#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "commands/CommandRegistry.hpp"
#include "commands/ExitCommand.hpp"
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
    const CommandResult exitError = ExitCommand{}.execute({"nope"});
    check(!exitError.shouldExit && !exitError.error.empty(), "exit rejects bad codes");

    const CommandResult unknown = CommandRegistry{}.execute("missing", {});
    check(unknown.error == "shell: unknown command: missing", "unknown command is reported");

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
