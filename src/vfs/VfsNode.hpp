#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

struct VfsNode
{
    std::string name;
    bool directory = false;
    std::vector<std::uint8_t> data;
    std::vector<std::unique_ptr<VfsNode>> children;

    VfsNode* findChild(std::string_view childName) const
    {
        for (const std::unique_ptr<VfsNode>& child : children)
        {
            if (child->name == childName)
            {
                return child.get();
            }
        }
        return nullptr;
    }

    VfsNode* ensureChild(std::string_view childName, bool childDirectory)
    {
        if (VfsNode* existing = findChild(childName))
        {
            return existing;
        }
        children.push_back(std::make_unique<VfsNode>());
        VfsNode& created = *children.back();
        created.name = std::string(childName);
        created.directory = childDirectory;
        return &created;
    }
};
