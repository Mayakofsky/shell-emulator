#pragma once

#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

class JsonLogger
{
public:
    explicit JsonLogger(const std::string& path);
    bool isOpen() const;
    const std::string& error() const;
    void log(const std::optional<std::string>& command, const std::vector<std::string>& arguments,
        std::string_view status);
    static std::string escape(std::string_view value);

private:
    static std::string currentTime();
    std::ofstream file_;
    std::string error_;
};
