#ifndef PGE_OS_OS_FILE_H
#define PGE_OS_OS_FILE_H

#include <string>
#include <fstream>
#include <diag_assert.h>

namespace pge
{
    inline std::string
    os_ReadFile(const char* path)
    {
        std::ifstream file(path);
        diag_Assert(file.is_open());
        return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }
} // namespace pge

#endif