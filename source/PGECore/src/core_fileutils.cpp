#include "../include/core_fileutils.h"
#include "../include/core_assert.h"

#include <fstream>
#include <filesystem>

namespace pge
{
    std::string
    core_ReadFile(const char* path)
    {
        std::ifstream file(path);
        core_Assert(file.is_open());
        return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }

    std::string
    core_GetFilenameFromPath(const char* path)
    {
        size_t len           = strlen(path);
        size_t lastSep = len;
        while (path[--lastSep] != '\\') {
            core_Assert(lastSep >= 0);
        }

        size_t      nameBeg = lastSep + 1;
        size_t      nameLen = len - nameBeg;
        std::string name;
        for (size_t i = 0; i < nameLen; ++i) {
            name += path[nameBeg + i];
        }
        return name;
    }

    std::string
    core_GetExtensionFromPath(const char* path)
    {
        size_t len     = strlen(path);
        size_t lastDot = len;
        while (path[--lastDot] != '.') {
            core_Assert(lastDot >= 0);
        }
        size_t      extBeg = lastDot + 1;
        size_t      extLen = len - extBeg;
        std::string ext;
        for (size_t i = 0; i < extLen; ++i) {
            ext += path[extBeg + i];
        }
        return ext;
    }

    std::vector<core_FSItem>
    core_FSItemsInDirectory(const char* dir)
    {
        std::vector<core_FSItem> items;
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            core_FSItem item;
            item.path = entry.path().generic_string();
            for (auto& c : item.path) {
                if (c == '/') {
                    c = '\\';
                }
            }
            item.type = entry.is_directory() ? core_FSItemType::DIRECTORY : core_FSItemType::FILE;
            items.push_back(item);
        }
        return items;
    }

    std::vector<core_FSItem>
    core_FSItemsWithExtension(const char* dir, const char* extension, bool recursive)
    {
        std::vector<core_FSItem> result;
        std::vector<core_FSItem> items = core_FSItemsInDirectory(dir);
        for (const core_FSItem& item : items) {
            if (item.type == core_FSItemType::FILE) {
                if (core_GetExtensionFromPath(item.path.c_str()) == extension) {
                    result.push_back(item);
                }
            } else if (item.type == core_FSItemType::DIRECTORY && recursive) {
                auto dirItems = core_FSItemsWithExtension(item.path.c_str(), extension, recursive);
                for (const auto& ditem : dirItems) {
                    result.push_back(ditem);
                }
            }
        }
        return result;
    }
}