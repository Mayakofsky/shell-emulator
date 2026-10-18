#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

class Base64
{
public:
    static std::string encode(const std::vector<std::uint8_t>& data);
    static bool decode(std::string_view text, std::vector<std::uint8_t>& data);
    static bool isBinary(const std::vector<std::uint8_t>& data);
};
