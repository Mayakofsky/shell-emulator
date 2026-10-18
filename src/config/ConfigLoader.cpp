#include "ConfigLoader.hpp"

#include "ConfigFileReader.hpp"

ConfigLoadResult ConfigLoader::load(const std::vector<std::string>& arguments) const
{
    const ArgsParseResult cli = ArgsParser{}.parse(arguments);
    if (!cli.success)
    {
        return ConfigLoadResult{{}, cli.error, false};
    }

    Configuration fileConfiguration;
    if (cli.configuration.configPath)
    {
        const ConfigFileResult file = ConfigFileReader{}.read(*cli.configuration.configPath);
        if (!file.success)
        {
            return ConfigLoadResult{{}, "shell: config error: " + file.error, false};
        }
        fileConfiguration = file.configuration;
    }

    return ConfigLoadResult{merge(fileConfiguration, cli.configuration), {}, true};
}

Configuration ConfigLoader::merge(const Configuration& fileConfiguration,
    const Configuration& cliConfiguration)
{
    Configuration result = fileConfiguration;
    result.configPath = cliConfiguration.configPath;
    if (cliConfiguration.vfsPath)
    {
        result.vfsPath = cliConfiguration.vfsPath;
    }
    if (cliConfiguration.logPath)
    {
        result.logPath = cliConfiguration.logPath;
    }
    if (cliConfiguration.scriptPath)
    {
        result.scriptPath = cliConfiguration.scriptPath;
    }
    return result;
}
