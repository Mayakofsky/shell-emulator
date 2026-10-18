#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "commands/CommandContext.hpp"
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
#include "vfs/Base64.hpp"
#include "vfs/Vfs.hpp"

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
    check(tokens.success && tokens.tokens.size() == 3, "lexer splits arguments");

    const LexerResult quotes = Lexer{}.tokenize("cmd 'one two' \"three four\"");
    check(quotes.tokens[1].value == "one two" && quotes.tokens[2].value == "three four",
        "lexer removes quotes");

    const ParseResult parsed = Parser{}.parse("ls\t-l 'folder name'");
    check(parsed.success && parsed.command == "ls", "parser reads the command");
    check(parsed.arguments == std::vector<std::string>({"-l", "folder name"}),
        "parser reads the arguments");
    check(!Parser{}.parse("ls \"unfinished").success, "parser reports syntax errors");

    const std::string prompt = PromptBuilder{}.build("alice", "host",
        std::filesystem::path("/home/alice"), std::filesystem::path("/home/alice"));
    check(prompt == "alice@host:~$ ", "prompt hides the home directory");

    const ArgsParseResult args = ArgsParser{}.parse(
        {"--vfs", "vfs-a", "--log=log-a", "--script", "script-a"});
    check(args.success && args.configuration.vfsPath == "vfs-a", "argument parser reads vfs");
    check(!ArgsParser{}.parse({"--bad", "x"}).success, "argument parser rejects unknown keys");

    const std::filesystem::path configPath =
        writeTemp("shell_emulator_test_config.xml",
            "<config><vfs>file-vfs</vfs><log>file-log</log></config>");
    const ConfigLoadResult loaded = ConfigLoader{}.load(
        {"--config", configPath.string(), "--vfs=cli-vfs"});
    std::filesystem::remove(configPath);
    check(loaded.success && loaded.configuration.vfsPath == "cli-vfs",
        "command line overrides the config file");

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

    const std::vector<std::uint8_t> binary = {0x00, 0x01, 0x7f, 0x80, 0xff};
    std::vector<std::uint8_t> decoded;
    check(Base64::decode(Base64::encode(binary), decoded) && decoded == binary,
        "base64 round trip");

    Vfs vfs;
    vfs.buildFromEntries({
        {"notes.txt", false, {'h', 'i'}},
        {"dir", true, {}},
        {"dir/file.txt", false, {'f'}}});
    std::string currentDirectory = "/";
    CommandContext context{vfs, currentDirectory};
    check(vfs.entryCount() == 3, "vfs builds a tree");
    check(vfs.resolve("/dir/./file.txt") != nullptr, "vfs resolves relative parts");
    check(vfs.resolve("/missing") == nullptr, "vfs reports missing paths");

    std::string listError;
    const std::vector<std::string> paths = vfs.listPaths("/", listError);
    check(paths == std::vector<std::string>({"/dir/", "/dir/file.txt", "/notes.txt"}),
        "vfs lists paths sorted");

    const CommandResult tree = CommandRegistry{}.execute(context, "vfs-tree", {});
    check(tree.output == "/dir/\n/dir/file.txt\n/notes.txt\n", "vfs-tree prints paths");
    const CommandResult read = CommandRegistry{}.execute(context, "vfs-read", {"/notes.txt"});
    check(read.output == "hi\n", "vfs-read prints text");
    const CommandResult readMissing = CommandRegistry{}.execute(
        context, "vfs-read", {"/absent"});
    check(!readMissing.error.empty(), "vfs-read reports errors");

    const CommandResult exitCode = ExitCommand{}.execute(context, {"-3"});
    check(exitCode.shouldExit && exitCode.exitCode == -3, "exit keeps the code");

    const CommandResult unknown = CommandRegistry{}.execute(context, "missing", {});
    check(unknown.error == "shell: unknown command: missing", "unknown command is reported");

    if (failures == 0)
    {
        std::cout << "all tests passed\n";
        return 0;
    }
    std::cout << failures << " test(s) failed\n";
    return 1;
}
