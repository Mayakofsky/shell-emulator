#pragma once

#include <cstdint>
#include <vector>

class Inflate
{
public:
    static bool decompress(const std::vector<std::uint8_t>& input,
        std::vector<std::uint8_t>& output);
};
