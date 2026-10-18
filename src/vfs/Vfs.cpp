#include "Vfs.hpp"

#include "ZipReader.hpp"

#include <algorithm>

namespace
{
std::string joinPath(const std::string& prefix, const std::string& name)
{
    if (prefix.empty() || prefix == "/")
    {
        return "/" + name;
    }
    return prefix + "/" + name;
}
}

Vfs::Vfs()
{
    root_.name = "/";
    root_.directory = true;
}

bool Vfs::loadFromArchive(const std::string& path, std::string& error)
{
    std::vector<ZipEntry> entries;
    if (!ZipReader::read(path, entries, error))
    {
        return false;
    }
    buildFromEntries(entries);
    return true;
}

void Vfs::buildFromEntries(const std::vector<ZipEntry>& entries)
{
    clear();
    for (const ZipEntry& entry : entries)
    {
        std::string name = entry.name;
        while (!name.empty() && name.front() == '/')
        {
            name.erase(name.begin());
        }
        while (!name.empty() && name.back() == '/')
        {
            name.pop_back();
        }
        if (name.empty())
        {
            continue;
        }

        const std::vector<std::string> components = splitPath(name);
        VfsNode* current = &root_;
        for (std::size_t index = 0; index + 1 < components.size(); ++index)
        {
            current = current->ensureChild(components[index], true);
            current->directory = true;
        }

        VfsNode* leaf = current->ensureChild(components.back(),
            entry.isDirectory);
        leaf->directory = entry.isDirectory;
        leaf->data = entry.rawBytes;
    }
}

void Vfs::clear()
{
    root_ = VfsNode{};
    root_.name = "/";
    root_.directory = true;
}

bool Vfs::isEmpty() const
{
    return root_.children.empty();
}

std::size_t Vfs::countNodes(const VfsNode& node)
{
    std::size_t total = 0;
    for (const std::unique_ptr<VfsNode>& child : node.children)
    {
        total += 1 + countNodes(*child);
    }
    return total;
}

std::size_t Vfs::entryCount() const
{
    return countNodes(root_);
}

const VfsNode* Vfs::root() const
{
    return &root_;
}

std::vector<std::string> Vfs::splitPath(const std::string& path)
{
    std::vector<std::string> components;
    std::string component;
    for (const char character : path)
    {
        if (character == '/' || character == '\\')
        {
            if (!component.empty())
            {
                components.push_back(component);
                component.clear();
            }
            continue;
        }
        component.push_back(character);
    }
    if (!component.empty())
    {
        components.push_back(component);
    }
    return components;
}

const VfsNode* Vfs::resolveCanonical(const std::string& path,
    std::string& canonical) const
{
    std::vector<std::string> resolved;
    const VfsNode* current = &root_;
    for (const std::string& component : splitPath(path))
    {
        if (component == ".")
        {
            continue;
        }
        if (component == "..")
        {
            if (current == &root_ || resolved.empty())
            {
                current = &root_;
                continue;
            }
            resolved.pop_back();
            current = &root_;
            for (const std::string& step : resolved)
            {
                const VfsNode* next = current->findChild(step);
                if (next == nullptr)
                {
                    canonical.clear();
                    return nullptr;
                }
                current = next;
            }
            continue;
        }

        const VfsNode* next = current->findChild(component);
        if (next == nullptr)
        {
            canonical.clear();
            return nullptr;
        }
        resolved.push_back(component);
        current = next;
    }

    canonical = "/";
    for (const std::string& component : resolved)
    {
        canonical = joinPath(canonical, component);
    }
    return current;
}

const VfsNode* Vfs::resolve(const std::string& path) const
{
    std::string canonical;
    return resolveCanonical(path, canonical);
}

void Vfs::collectSubtree(const VfsNode& node, const std::string& prefix,
    std::vector<std::string>& paths)
{
    for (const std::unique_ptr<VfsNode>& child : node.children)
    {
        const std::string childPath = joinPath(prefix, child->name);
        if (child->directory)
        {
            paths.push_back(childPath + "/");
            collectSubtree(*child, childPath, paths);
        }
        else
        {
            paths.push_back(childPath);
        }
    }
}

std::vector<std::string> Vfs::listPaths(const std::string& path,
    std::string& error) const
{
    error.clear();
    std::string canonical;
    const VfsNode* node = resolveCanonical(path, canonical);
    if (node == nullptr)
    {
        error = "no such path: " + path;
        return {};
    }
    if (!node->directory)
    {
        return {canonical};
    }

    std::vector<std::string> paths;
    collectSubtree(*node, canonical, paths);
    std::sort(paths.begin(), paths.end());
    return paths;
}
