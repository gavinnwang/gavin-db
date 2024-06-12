#pragma once

#include <cstdio>
#include <ctime>
#include <array>

namespace db {

// Utility to extract the file name
using cstr = const char* const;
constexpr cstr past_last_slash(cstr str, cstr last_slash) {
    return *str == '\0' ? last_slash
           : *str == '/' ? past_last_slash(str + 1, str + 1)
                         : past_last_slash(str + 1, last_slash);
}
constexpr cstr past_last_slash(cstr str) {
    return past_last_slash(str, str);
}

#define __SHORT_FILE__ ([] { constexpr cstr sf__ { past_last_slash(__FILE__) }; return sf__; }())

// Log levels
enum LogLevel {
    LOG_LEVEL_OFF = 1000,
    LOG_LEVEL_ERROR = 500,
    LOG_LEVEL_WARN = 400,
    LOG_LEVEL_INFO = 300,
    LOG_LEVEL_DEBUG = 200,
    LOG_LEVEL_TRACE = 100,
    LOG_LEVEL_ALL = 0
};

// Define the current log level
constexpr LogLevel CURRENT_LOG_LEVEL = LOG_LEVEL_ALL;

// Time format and output stream
#define LOG_LOG_TIME_FORMAT "%M:%S"
#define LOG_OUTPUT_STREAM stdout

// For compilers which do not support __FUNCTION__
#if !defined(__FUNCTION__) && !defined(__GNUC__)
#define __FUNCTION__ ""
#endif

void OutputLogHeader(const char* file, int line, const char* func, LogLevel level);

constexpr bool is_log_level_enabled(LogLevel level) {
    return level >= CURRENT_LOG_LEVEL;
}

template <LogLevel level, typename... Args>
void log_if_enabled(const char* file, int line, const char* func, const char* message, Args&&... args) {
    if constexpr (is_log_level_enabled(level)) {
        OutputLogHeader(file, line, func, level);
        if constexpr (!sizeof...(Args)) {
            // If no additional arguments, just print the message
            fprintf(LOG_OUTPUT_STREAM, "%s\n", message);
        } else {
            // Use a format string if there are additional arguments
            fprintf(LOG_OUTPUT_STREAM, message, std::forward<Args>(args)...);
            fprintf(LOG_OUTPUT_STREAM, "\n");
        }
        fflush(stdout);
    }
}

#define LOG_ERROR(...) log_if_enabled<LOG_LEVEL_ERROR>(__SHORT_FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_WARN(...) log_if_enabled<LOG_LEVEL_WARN>(__SHORT_FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_INFO(...) log_if_enabled<LOG_LEVEL_INFO>(__SHORT_FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_DEBUG(...) log_if_enabled<LOG_LEVEL_DEBUG>(__SHORT_FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_TRACE(...) log_if_enabled<LOG_LEVEL_TRACE>(__SHORT_FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

// Output log message header in this format: [type] [file:line:function] time -
// ex: [ERROR] [somefile.cpp:123:doSome()] 2008/07/06 10:00:00 -
inline void OutputLogHeader(const char* file, int line, const char* func, LogLevel level) {
    time_t t = time(nullptr);
    tm* curTime = localtime(&t); // NOLINT
    char time_str[32];
    strftime(time_str, 32, LOG_LOG_TIME_FORMAT, curTime);
    const char* type;
    switch (level) {
        case LOG_LEVEL_ERROR: type = "ERROR"; break;
        case LOG_LEVEL_WARN: type = "WARN "; break;
        case LOG_LEVEL_INFO: type = "INFO "; break;
        case LOG_LEVEL_DEBUG: type = "DEBUG"; break;
        case LOG_LEVEL_TRACE: type = "TRACE"; break;
        default: type = "UNKWN";
    }
    fprintf(LOG_OUTPUT_STREAM, "%s [%s:%d:%s] %s - ", time_str, file, line, func, type);
}

} // namespace db
