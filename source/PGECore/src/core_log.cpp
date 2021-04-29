#include "../include/core_log.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <iostream>

#ifdef _WIN32
#    include <Windows.h>
#endif

namespace pge
{
    std::vector<core_LogRecord> s_records;

    static void
    LogMessage(const char* tag, const char* message)
    {
        time_t t;
        time(&t);
        tm time{};
        localtime_s(&time, &t);

        // Format the string to match the form [dd/mm|hh:mm:ss - tag] %message%
        std::stringstream ss;
        ss << "[" << std::setfill('0') << std::setw(2) << time.tm_mday << "/" << std::setw(2) << (time.tm_mon + 1) << "|" << std::setw(2)
           << time.tm_hour << ":" << std::setw(2) << time.tm_min << ":" << std::setw(2) << time.tm_sec << " - " << tag << "] " << message << "\n";
        auto formatted_message = ss.str();
        std::cout << formatted_message << std::flush;
#ifdef _WIN32
        OutputDebugString(formatted_message.c_str());
#endif
#undef ERROR
        core_LogRecord::RecordType recordType;
        if (strcmp(tag, "DEBUG") == 0)
            recordType = core_LogRecord::RecordType::DEBUG;
        else if (strcmp(tag, "WARNING") == 0)
            recordType = core_LogRecord::RecordType::WARNING;
        else if (strcmp(tag, "ERROR") == 0)
            recordType = core_LogRecord::RecordType::ERROR;

        s_records.push_back(core_LogRecord(recordType, formatted_message));
    }

    static void
    LogMessageFormatted(const char* tag, const char* format, va_list list)
    {
        char         dummy;
        const size_t bufferSize = vsnprintf(&dummy, 1, format, list) + 1;
        auto         buffer     = std::unique_ptr<char[]>(new char[bufferSize]);
        vsnprintf(&buffer[0], bufferSize, format, list);
        LogMessage(tag, buffer.get());
    }

    core_LogRecord::core_LogRecord(RecordType type, const std::string& message)
        : type(type)
        , message(message)
    {}

    std::vector<core_LogRecord>
    core_GetLogRecords()
    {
        return s_records;
    }

    void
    core_ClearLogRecords()
    {
        s_records.clear();
    }

    void
    core_LogDebug(const char* msg)
    {
        LogMessage("DEBUG", msg);
    }

    void
    core_LogDebugf(const char* fmt, ...)
    {
        va_list list;
        va_start(list, fmt);
        LogMessageFormatted("DEBUG", fmt, list);
        va_end(list);
    }

    void
    core_LogWarning(const char* msg)
    {
        LogMessage("WARNING", msg);
    }

    void
    core_LogWarningf(const char* fmt, ...)
    {
        va_list list;
        va_start(list, fmt);
        LogMessageFormatted("WARNING", fmt, list);
        va_end(list);
    }

    void
    core_LogError(const char* msg)
    {
        LogMessage("ERROR", msg);
    }

    void
    core_LogErrorf(const char* fmt, ...)
    {
        va_list list;
        va_start(list, fmt);
        LogMessageFormatted("ERROR", fmt, list);
        va_end(list);
    }
} // namespace pge