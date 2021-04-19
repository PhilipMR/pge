#ifndef PGE_DIAGNOSTICS_DIAG_LOG_H
#define PGE_DIAGNOSTICS_DIAG_LOG_H

#include <string>
#include <vector>

#undef ERROR

namespace pge
{
    struct diag_LogRecord {
        enum RecordType
        {
            DEBUG,
            WARNING,
            ERROR
        } type;
        std::string message;
        diag_LogRecord(RecordType type, const std::string& message);
    };
    std::vector<diag_LogRecord> diag_GetLogRecords();
    void                        diag_ClearLogRecords();

    void diag_LogDebug(const char* msg);
    void diag_LogDebugf(const char* fmt, ...);

    void diag_LogWarning(const char* msg);
    void diag_LogWarningf(const char* fmt, ...);

    void diag_LogError(const char* msg);
    void diag_LogErrorf(const char* fmt, ...);


} // namespace pge

#endif