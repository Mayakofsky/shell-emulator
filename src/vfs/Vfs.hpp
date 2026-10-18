#pragma once

#include "VfsNode.hpp"
#include "ZipReader.hpp"

#include <cstddef>
#include <string>
#include <vector>

class Vfs
{
public:
    Vfs();
    bool loadFromArchive(const std::string& path, std::string& error);
    void buildFromEntries(const std::vector<ZipEntry>& entries);
    void clear();
    bool isEmpty() const;
    std::size_t entryCount() const;
    const VfsNode* resolve(const std::string& path) const;
    const VfsNode* resolveFrom(const std::string& base, const std::string& path,
        std::string& canonical) const;
    const VfsNode* root() const;
    std::vector<std::string> listPaths(const std::string& path,
        std::string& error) const;

    bool removeEntry(const std::string& base, const std::string& path,
        bool recursive, std::string& error);
    bool moveEntry(const std::string& base, const std::string& source,
        const std::string& destination, std::string& error);

private:
    static std::vector<std::string> splitPath(const std::string& path);
    static std::string combine(const std::string& base, const std::string& path);
    static void eraseChild(VfsNode& parent, const std::string& name);

    VfsNode* walk(const std::string& path, std::string& canonical);
    const VfsNode* resolveCanonical(const std::string& path,
        std::string& canonical) const;
    VfsNode* locateParent(const std::string& base, const std::string& path,
        VfsNode*& parent, std::string& leaf, std::string& canonical);
    static void collectSubtree(const VfsNode& node, const std::string& prefix,
        std::vector<std::string>& paths);
    static std::size_t countNodes(const VfsNode& node);

    VfsNode root_;
};
