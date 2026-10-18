#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "commands/CdCommand.hpp"
#include "commands/CommandContext.hpp"
#include "commands/CommandRegistry.hpp"
#include "commands/ExitCommand.hpp"
#include "commands/FindCommand.hpp"
#include "commands/LsCommand.hpp"
#include "commands/MvCommand.hpp"
#include "commands/RmCommand.hpp"
#include "commands/WcCommand.hpp"
#include "commands/WhoCommand.hpp"
#include "config/ArgsParser.hpp"
#include "config/ConfigLoader.hpp"
#include "log/JsonLogger.hpp"
#include "shell/Environment.hpp"
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
    check(Lexer{}.tokenize("  ls   first\tsecond ").tokens.size() == 3,
        "lexer splits arguments");
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
    std::string listError;
    check(vfs.listPaths("/", listError) ==
        std::vector<std::string>({"/dir/", "/dir/file.txt", "/notes.txt"}),
        "vfs lists paths sorted");

    const CommandResult tree = CommandRegistry{}.execute(context, "vfs-tree", {});
    check(tree.output == "/dir/\n/dir/file.txt\n/notes.txt\n", "vfs-tree prints paths");
    const CommandResult read = CommandRegistry{}.execute(context, "vfs-read", {"/notes.txt"});
    check(read.output == "hi\n", "vfs-read prints text");

    check(LsCommand{}.execute(context, {}).output == "dir\nnotes.txt\n", "ls lists names");
    check(LsCommand{}.execute(context, {"/dir"}).output == "file.txt\n", "ls reads a path");
    check(LsCommand{}.execute(context, {"/notes.txt"}).error.starts_with("ls: not a"),
        "ls rejects files");
    check(CdCommand{}.execute(context, {"dir"}).error.empty() && currentDirectory == "/dir",
        "cd changes the directory");
    check(CdCommand{}.execute(context, {".."}).error.empty() && currentDirectory == "/",
        "cd goes to the parent");

    check(WcCommand{}.execute(context, {"notes.txt"}).output == "0 1 2 notes.txt\n",
        "wc counts a file");
    check(WcCommand{}.execute(context, {"-l", "notes.txt"}).output == "0 notes.txt\n",
        "wc honours flags");

    check(FindCommand{}.execute(context, {"-type", "d"}).output == "/dir/\n",
        "find filters directories");
    check(WhoCommand{}.execute(context, {}).output ==
        Environment::username() + " " + Environment::hostname() + "\n", "who prints the user");

    Vfs editVfs;
    editVfs.buildFromEntries({
        {"a.txt", false, {'a'}},
        {"b.txt", false, {'b'}},
        {"dir", true, {}}});
    std::string editDirectory = "/";
    CommandContext editContext{editVfs, editDirectory};

    check(MvCommand{}.execute(editContext, {"/a.txt", "/c.txt"}).error.empty(),
        "mv renames a file");
    check(editVfs.resolve("/a.txt") == nullptr && editVfs.resolve("/c.txt") != nullptr,
        "mv moved the file");
    check(MvCommand{}.execute(editContext, {"/b.txt", "/dir"}).error.empty(),
        "mv moves into a directory");
    check(editVfs.resolve("/dir/b.txt") != nullptr, "mv keeps the name");
    check(MvCommand{}.execute(editContext, {"/dir", "/dir/sub"}).error.starts_with("mv: cannot"),
        "mv forbids moving into itself");
    check(MvCommand{}.execute(editContext, {}).error == "mv: missing file operand",
        "mv needs arguments");

    check(RmCommand{}.execute(editContext, {"/c.txt"}).error.empty(), "rm removes a file");
    check(editVfs.resolve("/c.txt") == nullptr, "rm really removes the file");
    check(RmCommand{}.execute(editContext, {"/dir"}).error.starts_with("rm: is a"),
        "rm protects directories");
    check(RmCommand{}.execute(editContext, {"-r", "/dir"}).error.empty(),
        "rm -r removes a directory");
    check(editVfs.resolve("/dir") == nullptr, "rm -r clears the subtree");
    check(RmCommand{}.execute(editContext, {"/"}).error.starts_with("rm: cannot remove root"),
        "rm protects the root");

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
