#include "shell/Shell.hpp"

#include "config/ConfigLoader.hpp"
#include "log/JsonLogger.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace
{
std::vector<std::string> argumentsOf(int argc, char* argv[])
{
    std::vector<std::string> arguments;
    for (int index = 1; index < argc; ++index)
    {
        arguments.emplace_back(argv[index]);
    }
    return arguments;
}

std::string valueOrUnset(const std::optional<std::string>& value)
{
    return value ? *value : "(unset)";
}

void printConfiguration(const Configuration& configuration)
{
    std::cout << "configuration:\n"
              << "  config = " << valueOrUnset(configuration.configPath) << '\n'
              << "  vfs    = " << valueOrUnset(configuration.vfsPath) << '\n'
              << "  log    = " << valueOrUnset(configuration.logPath) << '\n'
              << "  script = " << valueOrUnset(configuration.scriptPath) << '\n';
}
}

int main(int argc, char* argv[])
{
    Shell::installSignalHandler();
    const ConfigLoadResult loaded = ConfigLoader{}.load(argumentsOf(argc, argv));
    if (!loaded.success)
    {
        std::cerr << loaded.error << '\n';
        return 1;
    }
    printConfiguration(loaded.configuration);

    std::unique_ptr<JsonLogger> logger;
    if (loaded.configuration.logPath)
    {
        logger = std::make_unique<JsonLogger>(*loaded.configuration.logPath);
        if (!logger->isOpen())
        {
            std::cerr << "shell: log error: " << logger->error() << '\n';
            return 1;
        }
    }

    Shell shell(logger.get(), loaded.configuration.scriptPath);
    return shell.run();
}
