#include "JsonLogger.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

JsonLogger::JsonLogger(const std::string& path)
{
    file_.open(path, std::ios::out | std::ios::trunc);
    if (!file_)
    {
        error_ = "cannot open file: " + path;
    }
}

bool JsonLogger::isOpen() const
{
    return file_.is_open() && !file_.fail();
}

const std::string& JsonLogger::error() const
{
    return error_;
}

void JsonLogger::log(const std::optional<std::string>& command,
    const std::vector<std::string>& arguments,
    std::string_view status)
{
    if (!isOpen())
    {
        return;
    }
    file_ << "{\"time\":\"" << currentTime() << "\",\"command\":";
    if (command)
    {
        file_ << "\"" << escape(*command) << "\"";
    }
    else
    {
        file_ << "null";
    }
    file_ << ",\"args\":[";
    for (std::size_t index = 0; index < arguments.size(); ++index)
    {
        if (index > 0)
        {
            file_ << ',';
        }
        file_ << "\"" << escape(arguments[index]) << "\"";
    }
    file_ << "],\"status\":\"" << escape(status) << "\"}\n";
    file_.flush();
}

std::string JsonLogger::escape(std::string_view value)
{
    std::ostringstream output;
    output << std::hex << std::setfill('0');
    for (const unsigned char character : value)
    {
        if (character == '"')
        {
            output << "\\\"";
        }
        else if (character == '\\')
        {
            output << "\\\\";
        }
        else if (character < 0x20)
        {
            output << "\\u" << std::setw(4) << static_cast<int>(character);
        }
        else
        {
            output << static_cast<char>(character);
        }
    }
    return output.str();
}

std::string JsonLogger::currentTime()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm utc{};
    const std::tm* converted = std::gmtime(&time);
    if (converted != nullptr)
    {
        utc = *converted;
    }
    std::ostringstream output;
    output << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    return output.str();
}
