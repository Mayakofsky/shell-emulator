#include "Inflate.hpp"

#include <array>

namespace
{
constexpr std::size_t maxBits = 15;
constexpr std::size_t maxSymbols = 288;
constexpr std::size_t outputLimit = 1ull << 28;

constexpr std::array<std::uint16_t, 29> lengthBase = {
    3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
    35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
constexpr std::array<std::uint8_t, 29> lengthExtra = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
    3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
constexpr std::array<std::uint16_t, 30> distanceBase = {
    1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
    257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289,
    16385, 24577};
constexpr std::array<std::uint8_t, 30> distanceExtra = {
    0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
    7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};
constexpr std::array<std::uint8_t, 19> codeLengthOrder = {
    16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

struct HuffmanTable
{
    std::array<std::uint16_t, maxBits + 1> counts{};
    std::array<std::uint16_t, maxSymbols> symbols{};
};

class BitReader
{
public:
    explicit BitReader(const std::vector<std::uint8_t>& data)
        : data_(data)
    {
    }

    std::uint32_t bits(std::size_t count)
    {
        std::uint32_t value = 0;
        for (std::size_t index = 0; index < count; ++index)
        {
            value |= bit() << index;
        }
        return value;
    }

    void alignToByte()
    {
        bitPosition_ = (bitPosition_ + 7) / 8 * 8;
    }

    bool byte(std::uint8_t& value)
    {
        alignToByte();
        const std::size_t index = bitPosition_ / 8;
        if (index >= data_.size())
        {
            exhausted_ = true;
            return false;
        }
        value = data_[index];
        bitPosition_ += 8;
        return true;
    }

    std::size_t bytesLeft() const
    {
        const std::size_t index = (bitPosition_ + 7) / 8;
        return index < data_.size() ? data_.size() - index : 0;
    }

    bool exhausted() const
    {
        return exhausted_;
    }

private:
    std::uint32_t bit()
    {
        const std::size_t index = bitPosition_ / 8;
        if (index >= data_.size())
        {
            exhausted_ = true;
            return 0;
        }
        const std::uint32_t value =
            (data_[index] >> (bitPosition_ % 8)) & 1u;
        ++bitPosition_;
        return value;
    }

    const std::vector<std::uint8_t>& data_;
    std::size_t bitPosition_ = 0;
    bool exhausted_ = false;
};

bool buildTable(const std::vector<std::uint8_t>& lengths, HuffmanTable& table)
{
    table.counts.fill(0);
    table.symbols.fill(0);
    for (const std::uint8_t length : lengths)
    {
        if (length > maxBits)
        {
            return false;
        }
        ++table.counts[length];
    }
    if (table.counts[0] == lengths.size())
    {
        return true;
    }

    int left = 1;
    for (std::size_t length = 1; length <= maxBits; ++length)
    {
        left <<= 1;
        left -= table.counts[length];
        if (left < 0)
        {
            return false;
        }
    }

    std::array<std::uint16_t, maxBits + 1> offsets{};
    for (std::size_t length = 1; length < maxBits; ++length)
    {
        offsets[length + 1] =
            static_cast<std::uint16_t>(offsets[length] + table.counts[length]);
    }
    for (std::size_t symbol = 0; symbol < lengths.size() &&
        symbol < maxSymbols; ++symbol)
    {
        if (lengths[symbol] != 0)
        {
            table.symbols[offsets[lengths[symbol]]++] =
                static_cast<std::uint16_t>(symbol);
        }
    }
    return true;
}

int decodeSymbol(BitReader& reader, const HuffmanTable& table)
{
    int code = 0;
    int first = 0;
    int index = 0;
    for (std::size_t length = 1; length <= maxBits; ++length)
    {
        code |= static_cast<int>(reader.bits(1));
        if (reader.exhausted())
        {
            return -1;
        }
        const int count = table.counts[length];
        if (code - first < count)
        {
            return table.symbols[static_cast<std::size_t>(index + (code - first))];
        }
        index += count;
        first = (first + count) << 1;
        code <<= 1;
    }
    return -1;
}

bool inflateStored(BitReader& reader, std::vector<std::uint8_t>& output)
{
    std::uint8_t lengthLow = 0;
    std::uint8_t lengthHigh = 0;
    std::uint8_t complementLow = 0;
    std::uint8_t complementHigh = 0;
    if (!reader.byte(lengthLow) || !reader.byte(lengthHigh) ||
        !reader.byte(complementLow) || !reader.byte(complementHigh))
    {
        return false;
    }
    const std::uint16_t length =
        static_cast<std::uint16_t>(lengthLow | static_cast<std::uint16_t>(lengthHigh << 8));
    const std::uint16_t complement =
        static_cast<std::uint16_t>(complementLow |
            static_cast<std::uint16_t>(complementHigh << 8));
    if (static_cast<std::uint16_t>(~length) != complement ||
        reader.bytesLeft() < length)
    {
        return false;
    }
    for (std::uint16_t index = 0; index < length; ++index)
    {
        std::uint8_t value = 0;
        if (!reader.byte(value))
        {
            return false;
        }
        output.push_back(value);
    }
    return true;
}

bool buildFixedTables(HuffmanTable& literal, HuffmanTable& distance)
{
    std::vector<std::uint8_t> lengths(288, 8);
    for (std::size_t symbol = 0; symbol < 144; ++symbol)
    {
        lengths[symbol] = 8;
    }
    for (std::size_t symbol = 144; symbol < 256; ++symbol)
    {
        lengths[symbol] = 9;
    }
    for (std::size_t symbol = 256; symbol < 280; ++symbol)
    {
        lengths[symbol] = 7;
    }
    for (std::size_t symbol = 280; symbol < 288; ++symbol)
    {
        lengths[symbol] = 8;
    }
    if (!buildTable(lengths, literal))
    {
        return false;
    }
    return buildTable(std::vector<std::uint8_t>(30, 5), distance);
}

bool buildDynamicTables(BitReader& reader, HuffmanTable& literal,
    HuffmanTable& distance)
{
    const std::size_t literalCount = 257 + reader.bits(5);
    const std::size_t distanceCount = 1 + reader.bits(5);
    const std::size_t codeCount = 4 + reader.bits(4);
    if (reader.exhausted() || literalCount > 286 || distanceCount > 30)
    {
        return false;
    }

    std::vector<std::uint8_t> codeLengths(19, 0);
    for (std::size_t index = 0; index < codeCount; ++index)
    {
        codeLengths[codeLengthOrder[index]] =
            static_cast<std::uint8_t>(reader.bits(3));
    }
    HuffmanTable codeTable;
    if (reader.exhausted() || !buildTable(codeLengths, codeTable))
    {
        return false;
    }

    std::vector<std::uint8_t> lengths(literalCount + distanceCount, 0);
    std::size_t index = 0;
    while (index < lengths.size())
    {
        const int symbol = decodeSymbol(reader, codeTable);
        if (symbol < 0)
        {
            return false;
        }
        if (symbol < 16)
        {
            lengths[index++] = static_cast<std::uint8_t>(symbol);
            continue;
        }

        std::uint8_t value = 0;
        std::size_t repeat = 0;
        if (symbol == 16)
        {
            if (index == 0)
            {
                return false;
            }
            value = lengths[index - 1];
            repeat = 3 + reader.bits(2);
        }
        else if (symbol == 17)
        {
            repeat = 3 + reader.bits(3);
        }
        else
        {
            repeat = 11 + reader.bits(7);
        }
        if (reader.exhausted() || index + repeat > lengths.size())
        {
            return false;
        }
        while (repeat-- > 0)
        {
            lengths[index++] = value;
        }
    }

    const std::vector<std::uint8_t> literalLengths(
        lengths.begin(),
        lengths.begin() + static_cast<std::ptrdiff_t>(literalCount));
    const std::vector<std::uint8_t> distanceLengths(
        lengths.begin() + static_cast<std::ptrdiff_t>(literalCount),
        lengths.end());
    return buildTable(literalLengths, literal) &&
        buildTable(distanceLengths, distance);
}

bool copyMatch(BitReader& reader, std::vector<std::uint8_t>& output,
    const HuffmanTable& distance, int symbol)
{
    if (symbol < 257 || symbol > 285 || output.size() > outputLimit)
    {
        return false;
    }
    const std::size_t lengthIndex = static_cast<std::size_t>(symbol - 257);
    std::size_t length = lengthBase[lengthIndex];
    if (lengthExtra[lengthIndex] != 0)
    {
        length += reader.bits(lengthExtra[lengthIndex]);
    }

    const int distanceSymbol = decodeSymbol(reader, distance);
    if (distanceSymbol < 0 || distanceSymbol > 29)
    {
        return false;
    }
    const std::size_t distanceIndex = static_cast<std::size_t>(distanceSymbol);
    std::size_t distanceValue = distanceBase[distanceIndex];
    if (distanceExtra[distanceIndex] != 0)
    {
        distanceValue += reader.bits(distanceExtra[distanceIndex]);
    }
    if (reader.exhausted() || distanceValue == 0 ||
        distanceValue > output.size())
    {
        return false;
    }
    for (std::size_t index = 0; index < length; ++index)
    {
        output.push_back(output[output.size() - distanceValue]);
    }
    return true;
}

bool inflateBlock(BitReader& reader, std::vector<std::uint8_t>& output,
    const HuffmanTable& literal, const HuffmanTable& distance)
{
    while (true)
    {
        const int symbol = decodeSymbol(reader, literal);
        if (symbol < 0)
        {
            return false;
        }
        if (symbol == 256)
        {
            return true;
        }
        if (symbol < 256)
        {
            if (output.size() >= outputLimit)
            {
                return false;
            }
            output.push_back(static_cast<std::uint8_t>(symbol));
            continue;
        }
        if (!copyMatch(reader, output, distance, symbol))
        {
            return false;
        }
    }
}
}

bool Inflate::decompress(const std::vector<std::uint8_t>& input,
    std::vector<std::uint8_t>& output)
{
    output.clear();
    if (input.empty())
    {
        return false;
    }

    BitReader reader(input);
    bool lastBlock = false;
    do
    {
        lastBlock = reader.bits(1) != 0;
        const std::uint32_t type = reader.bits(2);
        if (reader.exhausted())
        {
            return false;
        }
        if (type == 0)
        {
            if (!inflateStored(reader, output))
            {
                return false;
            }
        }
        else if (type == 1 || type == 2)
        {
            HuffmanTable literal;
            HuffmanTable distance;
            const bool built = type == 1
                ? buildFixedTables(literal, distance)
                : buildDynamicTables(reader, literal, distance);
            if (!built || !inflateBlock(reader, output, literal, distance))
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    } while (!lastBlock && !reader.exhausted());
    return lastBlock && !reader.exhausted();
}
