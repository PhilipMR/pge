#ifndef PGE_OS_OS_FILE_H
#define PGE_OS_OS_FILE_H

#include <string>
#include <fstream>
#include <vector>
#include <diag_assert.h>
#include <filesystem>

namespace pge
{
    inline std::string
    os_ReadFile(const char* path)
    {
        std::ifstream file(path);
        diag_Assert(file.is_open());
        return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }

    enum class os_ListItemType
    {
        FILE,
        DIRECTORY
    };

    struct os_ListItem {
        os_ListItemType type;
        std::string     path;
    };

    inline std::vector<os_ListItem>
    os_ListItemsInDirectory(const char* dir)
    {
        std::vector<os_ListItem> items;
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            os_ListItem item;
            item.path = entry.path().generic_string();
            for (auto& c : item.path) {
                if (c == '/') {
                    c = '\\';
                }
            }
            item.type = entry.is_directory() ? os_ListItemType::DIRECTORY : os_ListItemType::FILE;
            items.push_back(item);
        }
        return items;
    }

    inline std::string
    os_GetFileExtension(const char* path)
    {
        size_t len = strlen(path);
        size_t lastDot = len;
        while (path[--lastDot] != '.') {
            diag_Assert(lastDot >= 0);
        }
        size_t extBeg = lastDot + 1;
        size_t extLen = len - extBeg;
        std::string ext;
        for (size_t i = 0; i < extLen; ++i) {
            ext += path[extBeg+i];
        }
        return ext;
    }

    inline std::vector<os_ListItem>
    os_ListItemsWithExtension(const char* dir, const char* extension, bool recursive)
    {
        std::vector<os_ListItem> result;
        std::vector<os_ListItem> items = os_ListItemsInDirectory(dir);
        for (const os_ListItem& item : items) {
            if (item.type == os_ListItemType::FILE) {
                if (os_GetFileExtension(item.path.c_str()) == extension) {
                    result.push_back(item);
                }
            } else if (item.type == os_ListItemType::DIRECTORY && recursive) {
                auto dirItems = os_ListItemsWithExtension(item.path.c_str(), extension, recursive);
                for (const auto& ditem : dirItems) {
                    result.push_back(ditem);
                }
            }
        }
        return result;
    }
} // namespace pge

#endif