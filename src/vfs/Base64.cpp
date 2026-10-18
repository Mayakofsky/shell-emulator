#include "Base64.hpp"

#include <array>

namespace
{
constexpr std::string_view alphabet =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::array<int, 256> buildDecodeTable()
{
    std::array<int, 256> table{};
    table.fill(-1);
    for (std::size_t index = 0; index < alphabet.size(); ++index)
    {
        table[static_cast<unsigned char>(alphabet[index])] =
            static_cast<int>(index);
    }
    return table;
}

const std::array<int, 256> decodeTable = buildDecodeTable();
}

std::string Base64::encode(const std::vector<std::uint8_t>& data)
{
    std::string result;
    result.reserve((data.size() + 2) / 3 * 4);
    for (std::size_t index = 0; index < data.size(); index += 3)
    {
        const std::uint32_t chunk =
            (static_cast<std::uint32_t>(data[index]) << 16) |
            ((index + 1 < data.size()
                 ? static_cast<std::uint32_t>(data[index + 1]) : 0u) << 8) |
            (index + 2 < data.size()
                 ? static_cast<std::uint32_t>(data[index + 2]) : 0u);
        result.push_back(alphabet[(chunk >> 18) & 0x3f]);
        result.push_back(alphabet[(chunk >> 12) & 0x3f]);
        result.push_back(index + 1 < data.size()
            ? alphabet[(chunk >> 6) & 0x3f] : '=');
        result.push_back(index + 2 < data.size()
            ? alphabet[chunk & 0x3f] : '=');
    }
    return result;
}

bool Base64::decode(std::string_view text, std::vector<std::uint8_t>& data)
{
    data.clear();
    if (text.empty())
    {
        return true;
    }
    if (text.size() % 4 != 0)
    {
        return false;
    }

    std::size_t padding = 0;
    while (padding < 2 && padding < text.size() &&
        text[text.size() - 1 - padding] == '=')
    {
        ++padding;
    }
    const std::size_t body = text.size() - padding;
    for (std::size_t index = 0; index < body; ++index)
    {
        if (decodeTable[static_cast<unsigned char>(text[index])] < 0)
        {
            return false;
        }
    }

    std::uint32_t buffer = 0;
    std::size_t buffered = 0;
    for (std::size_t index = 0; index < body; ++index)
    {
        buffer = (buffer << 6) |
            static_cast<std::uint32_t>(
                decodeTable[static_cast<unsigned char>(text[index])]);
        ++buffered;
        if (buffered == 4)
        {
            data.push_back(static_cast<std::uint8_t>(buffer >> 16));
            data.push_back(static_cast<std::uint8_t>(buffer >> 8));
            data.push_back(static_cast<std::uint8_t>(buffer));
            buffer = 0;
            buffered = 0;
        }
    }

    if (buffered == 1)
    {
        return false;
    }
    if (buffered == 2)
    {
        data.push_back(static_cast<std::uint8_t>(buffer >> 4));
    }
    else if (buffered == 3)
    {
        data.push_back(static_cast<std::uint8_t>(buffer >> 10));
        data.push_back(static_cast<std::uint8_t>(buffer >> 2));
    }
    return true;
}

bool Base64::isBinary(const std::vector<std::uint8_t>& data)
{
    for (const std::uint8_t byte : data)
    {
        if (byte == 0x00 || byte >= 0x80)
        {
            return true;
        }
    }
    return false;
}
