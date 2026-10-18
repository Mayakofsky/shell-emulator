#include "Environment.hpp"

#include <array>
#include <cstdlib>
#include <system_error>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <unistd.h>
#endif

std::optional<std::string> Environment::getVariable(std::string_view name)
{
    const std::string key(name);
    const char* value = std::getenv(key.c_str());
    if (value == nullptr)
    {
        return std::nullopt;
    }
    return std::string(value);
}

std::string Environment::username()
{
    const auto user = getVariable("USER");
    if (user && !user->empty())
    {
        return *user;
    }

    const auto userName = getVariable("USERNAME");
    if (userName && !userName->empty())
    {
        return *userName;
    }
    return "unknown";
}

std::string Environment::hostname()
{
    std::array<char, 256> host{};
    int result = -1;

#ifdef _WIN32
    WSADATA data{};
    if (WSAStartup(MAKEWORD(2, 2), &data) == 0)
    {
        result = gethostname(host.data(), static_cast<int>(host.size()));
        WSACleanup();
    }
#else
    result = gethostname(host.data(), host.size());
#endif

    host.back() = '\0';
    if (result == 0 && host.front() != '\0')
    {
        return std::string(host.data());
    }

    const auto computerName = getVariable("COMPUTERNAME");
    if (computerName && !computerName->empty())
    {
        return *computerName;
    }
    return "unknown";
}

std::filesystem::path Environment::homeDirectory()
{
    const auto home = getVariable("HOME");
    if (home && !home->empty())
    {
        return std::filesystem::path(*home);
    }

#ifdef _WIN32
    const auto userProfile = getVariable("USERPROFILE");
    if (userProfile && !userProfile->empty())
    {
        return std::filesystem::path(*userProfile);
    }
#endif

    return {};
}

std::filesystem::path Environment::currentDirectory()
{
    std::error_code error;
    return std::filesystem::current_path(error);
}
