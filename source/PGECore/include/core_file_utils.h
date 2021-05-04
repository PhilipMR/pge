#ifndef PGE_CORE_CORE_FILEUTILS_H
#define PGE_CORE_CORE_FILEUTILS_H

#include <string>
#include <vector>

namespace pge
{
    std::string core_ReadFile(const char* path);
    std::string core_GetFilenameFromPath(const char* path);
    std::string core_GetDirnameFromPath(const char* path);
    std::string core_GetExtensionFromPath(const char* path);

    enum class core_FSItemType
    {
        FILE,
        DIRECTORY
    };
    struct core_FSItem {
        core_FSItemType type;
        std::string     path;
    };
    std::vector<core_FSItem> core_FSItemsInDirectory(const char* dir);
    std::vector<core_FSItem> core_FSItemsWithExtension(const char* dir, const char* extension, bool recursive);
} // namespace pge

#endif