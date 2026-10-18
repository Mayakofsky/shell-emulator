#include "ZipReader.hpp"

#include "Inflate.hpp"

#include <cstring>
#include <fstream>

namespace
{
constexpr std::uint32_t localSignature = 0x04034b50;
constexpr std::uint32_t centralSignature = 0x02014b50;
constexpr std::uint32_t endSignature = 0x06054b50;
constexpr std::size_t localHeaderSize = 30;
constexpr std::size_t centralHeaderSize = 46;
constexpr std::size_t endHeaderSize = 22;
constexpr std::size_t endCommentMax = 0xffff;
constexpr std::uint16_t methodStore = 0;
constexpr std::uint16_t methodDeflate = 8;

std::uint16_t read16(const std::vector<std::uint8_t>& bytes, std::size_t offset)
{
    return static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(bytes[offset]) |
        (static_cast<std::uint16_t>(bytes[offset + 1]) << 8));
}

std::uint32_t read32(const std::vector<std::uint8_t>& bytes, std::size_t offset)
{
    return static_cast<std::uint32_t>(bytes[offset]) |
        (static_cast<std::uint32_t>(bytes[offset + 1]) << 8) |
        (static_cast<std::uint32_t>(bytes[offset + 2]) << 16) |
        (static_cast<std::uint32_t>(bytes[offset + 3]) << 24);
}

bool findEndRecord(const std::vector<std::uint8_t>& bytes, std::size_t& offset)
{
    if (bytes.size() < endHeaderSize)
    {
        return false;
    }
    const std::size_t lowest = bytes.size() - endHeaderSize < endCommentMax
        ? 0
        : bytes.size() - endHeaderSize - endCommentMax;
    for (std::size_t position = bytes.size() - endHeaderSize;
         position >= lowest; --position)
    {
        if (read32(bytes, position) == endSignature)
        {
            offset = position;
            return true;
        }
        if (position == 0)
        {
            break;
        }
    }
    return false;
}

struct CentralHeader
{
    std::uint16_t method = 0;
    std::uint32_t compressedSize = 0;
    std::uint32_t uncompressedSize = 0;
    std::uint32_t localOffset = 0;
    std::string name;
    bool directory = false;
};

bool parseCentralHeader(const std::vector<std::uint8_t>& bytes,
    std::size_t& offset, CentralHeader& header, std::string& error)
{
    if (offset + centralHeaderSize > bytes.size() ||
        read32(bytes, offset) != centralSignature)
    {
        error = "corrupt central directory";
        return false;
    }

    header.method = read16(bytes, offset + 10);
    header.compressedSize = read32(bytes, offset + 20);
    header.uncompressedSize = read32(bytes, offset + 24);
    const std::uint16_t nameLength = read16(bytes, offset + 28);
    const std::uint16_t extraLength = read16(bytes, offset + 30);
    const std::uint16_t commentLength = read16(bytes, offset + 32);
    header.localOffset = read32(bytes, offset + 42);
    if (offset + centralHeaderSize + nameLength > bytes.size())
    {
        error = "corrupt central directory";
        return false;
    }
    header.name.assign(bytes.begin() +
            static_cast<std::ptrdiff_t>(offset + centralHeaderSize),
        bytes.begin() +
            static_cast<std::ptrdiff_t>(offset + centralHeaderSize + nameLength));
    header.directory = !header.name.empty() && header.name.back() == '/';
    offset += centralHeaderSize + nameLength + extraLength + commentLength;
    return true;
}

bool extractStored(const std::vector<std::uint8_t>& bytes,
    const CentralHeader& header, std::size_t dataOffset,
    std::vector<std::uint8_t>& data, std::string& error)
{
    if (dataOffset + header.compressedSize > bytes.size())
    {
        error = "truncated entry data: " + header.name;
        return false;
    }
    data.assign(bytes.begin() + static_cast<std::ptrdiff_t>(dataOffset),
        bytes.begin() +
            static_cast<std::ptrdiff_t>(dataOffset + header.compressedSize));
    if (data.size() != header.uncompressedSize)
    {
        error = "size mismatch for entry: " + header.name;
        return false;
    }
    return true;
}

bool extractDeflated(const std::vector<std::uint8_t>& bytes,
    const CentralHeader& header, std::size_t dataOffset,
    std::vector<std::uint8_t>& data, std::string& error)
{
    if (dataOffset + header.compressedSize > bytes.size())
    {
        error = "truncated entry data: " + header.name;
        return false;
    }
    const std::vector<std::uint8_t> compressed(
        bytes.begin() + static_cast<std::ptrdiff_t>(dataOffset),
        bytes.begin() +
            static_cast<std::ptrdiff_t>(dataOffset + header.compressedSize));
    if (!Inflate::decompress(compressed, data))
    {
        error = "inflate failed for entry: " + header.name;
        return false;
    }
    if (data.size() != header.uncompressedSize)
    {
        error = "size mismatch for entry: " + header.name;
        return false;
    }
    return true;
}

bool extractEntry(const std::vector<std::uint8_t>& bytes,
    const CentralHeader& header, ZipEntry& entry, std::string& error)
{
    if (header.localOffset + localHeaderSize > bytes.size() ||
        read32(bytes, header.localOffset) != localSignature)
    {
        error = "corrupt local header for entry: " + header.name;
        return false;
    }
    const std::uint16_t localNameLength =
        read16(bytes, header.localOffset + 26);
    const std::uint16_t localExtraLength =
        read16(bytes, header.localOffset + 28);
    const std::size_t dataOffset = header.localOffset + localHeaderSize +
        localNameLength + localExtraLength;
    if (dataOffset > bytes.size())
    {
        error = "corrupt local header for entry: " + header.name;
        return false;
    }

    entry.name = header.name;
    entry.isDirectory = header.directory;
    if (header.compressedSize == 0 && header.uncompressedSize == 0)
    {
        entry.rawBytes.clear();
        return true;
    }
    if (header.method == methodStore)
    {
        return extractStored(bytes, header, dataOffset, entry.rawBytes, error);
    }
    if (header.method == methodDeflate)
    {
        return extractDeflated(bytes, header, dataOffset, entry.rawBytes, error);
    }
    error = "unsupported compression method: " + std::to_string(header.method);
    return false;
}
}

bool ZipReader::read(const std::string& path, std::vector<ZipEntry>& entries,
    std::string& error)
{
    std::ifstream input(path, std::ios::binary);
    if (!input)
    {
        error = "cannot read file: " + path;
        return false;
    }
    std::vector<std::uint8_t> bytes(
        (std::istreambuf_iterator<char>(input)),
        std::istreambuf_iterator<char>());
    return parse(bytes, entries, error);
}

bool ZipReader::parse(const std::vector<std::uint8_t>& bytes,
    std::vector<ZipEntry>& entries, std::string& error)
{
    entries.clear();
    std::size_t endOffset = 0;
    if (!findEndRecord(bytes, endOffset))
    {
        error = "not a zip archive: missing end of central directory";
        return false;
    }
    if (endOffset + endHeaderSize > bytes.size())
    {
        error = "not a zip archive: truncated end of central directory";
        return false;
    }

    const std::uint16_t totalEntries = read16(bytes, endOffset + 10);
    std::size_t offset = read32(bytes, endOffset + 16);
    for (std::uint16_t index = 0; index < totalEntries; ++index)
    {
        CentralHeader header;
        if (!parseCentralHeader(bytes, offset, header, error))
        {
            return false;
        }
        ZipEntry entry;
        if (!extractEntry(bytes, header, entry, error))
        {
            return false;
        }
        entries.push_back(std::move(entry));
    }
    return true;
}
