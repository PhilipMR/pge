#ifndef PGE_CORE_CORE_LOG_H
#define PGE_CORE_CORE_LOG_H

#include <string>
#include <vector>

#undef ERROR

namespace pge
{
    struct core_LogRecord {
        enum RecordType
        {
            DEBUG,
            WARNING,
            ERROR
        } type;
        std::string message;
        core_LogRecord(RecordType type, const std::string& message);
    };
    std::vector<core_LogRecord> core_GetLogRecords();
    void                        core_ClearLogRecords();

    void core_LogDebug(const char* msg);
    void core_LogDebugf(const char* fmt, ...);

    void core_LogWarning(const char* msg);
    void core_LogWarningf(const char* fmt, ...);

    void core_LogError(const char* msg);
    void core_LogErrorf(const char* fmt, ...);


} // namespace pge

#endif