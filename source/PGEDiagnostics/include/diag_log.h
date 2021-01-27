#ifndef PGE_DIAGNOSTICS_DIAG_LOG_H
#define PGE_DIAGNOSTICS_DIAG_LOG_H

namespace pge
{
    void diag_LogDebug(const char* msg);
    void diag_LogDebugf(const char* fmt, ...);

    void diag_LogWarning(const char* msg);
    void diag_LogWarningf(const char* fmt, ...);

    void diag_LogError(const char* msg);
    void diag_LogErrorf(const char* fmt, ...);
}

#endif