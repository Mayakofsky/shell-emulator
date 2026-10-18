#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct ZipEntry
{
    std::string name;
    bool isDirectory = false;
    std::vector<std::uint8_t> rawBytes;
};

class ZipReader
{
public:
    static bool read(const std::string& path, std::vector<ZipEntry>& entries,
        std::string& error);
    static bool parse(const std::vector<std::uint8_t>& bytes,
        std::vector<ZipEntry>& entries, std::string& error);
};
