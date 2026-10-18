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

std::unique_ptr<VfsNode> takeChild(VfsNode& parent, const std::string& name)
{
    std::vector<std::unique_ptr<VfsNode>>& children = parent.children;
    for (std::vector<std::unique_ptr<VfsNode>>::iterator it = children.begin();
        it != children.end(); ++it)
    {
        if ((*it)->name == name)
        {
            std::unique_ptr<VfsNode> detached = std::move(*it);
            children.erase(it);
            return detached;
        }
    }
    return nullptr;
}

bool isInside(const std::string& directory, const std::string& path)
{
    return path.size() > directory.size() + 1 &&
        path.compare(0, directory.size(), directory) == 0 &&
        path[directory.size()] == '/';
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

VfsNode* Vfs::walk(const std::string& path, std::string& canonical)
{
    std::vector<std::string> resolved;
    VfsNode* current = &root_;
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
                VfsNode* next = current->findChild(step);
                if (next == nullptr)
                {
                    canonical.clear();
                    return nullptr;
                }
                current = next;
            }
            continue;
        }

        VfsNode* next = current->findChild(component);
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

const VfsNode* Vfs::resolveCanonical(const std::string& path,
    std::string& canonical) const
{
    return const_cast<Vfs*>(this)->walk(path, canonical);
}

std::string Vfs::combine(const std::string& base, const std::string& path)
{
    if (!path.empty() && path.front() == '/')
    {
        return path;
    }
    if (base.empty() || base == "/")
    {
        return "/" + path;
    }
    return base + "/" + path;
}

const VfsNode* Vfs::resolve(const std::string& path) const
{
    std::string canonical;
    return resolveCanonical(path, canonical);
}

const VfsNode* Vfs::resolveFrom(const std::string& base, const std::string& path,
    std::string& canonical) const
{
    return resolveCanonical(combine(base, path), canonical);
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

VfsNode* Vfs::locateParent(const std::string& base, const std::string& path,
    VfsNode*& parent, std::string& leaf, std::string& canonical)
{
    const std::vector<std::string> components = splitPath(combine(base, path));
    if (components.empty())
    {
        parent = &root_;
        leaf.clear();
        canonical = "/";
        return &root_;
    }

    leaf = components.back();
    if (leaf == "." || leaf == "..")
    {
        parent = nullptr;
        leaf.clear();
        canonical.clear();
        return nullptr;
    }

    std::string parentPath = "/";
    for (std::size_t index = 0; index + 1 < components.size(); ++index)
    {
        parentPath = joinPath(parentPath, components[index]);
    }

    std::string parentCanonical;
    parent = walk(parentPath, parentCanonical);
    if (parent == nullptr || !parent->directory)
    {
        parent = nullptr;
        leaf.clear();
        canonical.clear();
        return nullptr;
    }

    canonical = joinPath(parentCanonical, leaf);
    return parent;
}

bool Vfs::removeEntry(const std::string& base, const std::string& path,
    bool recursive, std::string& error)
{
    error.clear();
    VfsNode* parent = nullptr;
    std::string leaf;
    std::string canonical;
    VfsNode* located = locateParent(base, path, parent, leaf, canonical);
    if (located == nullptr)
    {
        error = "no such file: " + path;
        return false;
    }
    if (leaf.empty())
    {
        error = "cannot remove root: /";
        return false;
    }

    VfsNode* child = parent->findChild(leaf);
    if (child == nullptr)
    {
        error = "no such file: " + path;
        return false;
    }
    if (child->directory && !recursive)
    {
        error = "is a directory: " + path;
        return false;
    }

    takeChild(*parent, leaf);
    return true;
}

bool Vfs::moveEntry(const std::string& base, const std::string& source,
    const std::string& destination, std::string& error)
{
    error.clear();
    VfsNode* sourceParent = nullptr;
    std::string sourceLeaf;
    std::string sourceCanonical;
    VfsNode* located = locateParent(base, source, sourceParent, sourceLeaf,
        sourceCanonical);
    if (located == nullptr)
    {
        error = "no such file: " + source;
        return false;
    }
    if (sourceLeaf.empty())
    {
        error = "cannot move root: /";
        return false;
    }

    VfsNode* node = sourceParent->findChild(sourceLeaf);
    if (node == nullptr)
    {
        error = "no such file: " + source;
        return false;
    }

    std::string destinationCanonical;
    VfsNode* destinationNode = walk(combine(base, destination),
        destinationCanonical);

    VfsNode* destinationParent = nullptr;
    std::string destinationLeaf;
    std::string targetCanonical;
    if (destinationNode != nullptr && destinationNode->directory)
    {
        destinationParent = destinationNode;
        destinationLeaf = sourceLeaf;
        targetCanonical = joinPath(destinationCanonical, sourceLeaf);
    }
    else
    {
        VfsNode* parentLocated = locateParent(base, destination, destinationParent,
            destinationLeaf, targetCanonical);
        if (parentLocated == nullptr || destinationLeaf.empty())
        {
            error = "invalid target: " + destination;
            return false;
        }
    }

    if (targetCanonical == sourceCanonical)
    {
        return true;
    }
    if (node->directory && isInside(sourceCanonical, targetCanonical))
    {
        error = "cannot move a directory into itself: " + source;
        return false;
    }

    std::unique_ptr<VfsNode> detached = takeChild(*sourceParent, sourceLeaf);
    if (detached == nullptr)
    {
        error = "no such file: " + source;
        return false;
    }
    takeChild(*destinationParent, destinationLeaf);
    detached->name = destinationLeaf;
    destinationParent->children.push_back(std::move(detached));
    return true;
}
