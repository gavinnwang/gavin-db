#pragma once

#include <array>
#include <cstdio>
#include <ctime>
#include <sstream>
#include <string>

namespace db {

// Log levels
enum class LogLevel { OFF = 1000, ERROR = 500, WARN = 400, INFO = 300, DEBUG = 200, TRACE = 100, ALL = 0 };

// Define the current log level
constexpr LogLevel CURRENT_LOG_LEVEL = LogLevel::TRACE;

// Utility to extract the file name
using cstr = const char *const;
constexpr cstr past_last_slash(cstr str, cstr last_slash) {
	return *str == '\0'  ? last_slash
	       : *str == '/' ? past_last_slash(str + 1, str + 1)
	                     : past_last_slash(str + 1, last_slash);
}
constexpr cstr past_last_slash(cstr str) {
	return past_last_slash(str, str);
}

#define __SHORT_FILE__                                                                                                 \
	([] {                                                                                                              \
		constexpr cstr sf__ {past_last_slash(__FILE__)};                                                               \
		return sf__;                                                                                                   \
	}())

// Time format and output stream
#define LOG_LOG_TIME_FORMAT "%M:%S"
#define LOG_OUTPUT_STREAM   stdout

// For compilers which do not support __FUNCTION__
#if !defined(__FUNCTION__) && !defined(__GNUC__)
#define __FUNCTION__ ""
#endif

void OutputLogHeader(const char *file, int line, const char *func, LogLevel level);

constexpr bool is_log_level_enabled(LogLevel level) {
	return level >= CURRENT_LOG_LEVEL;
}

template <LogLevel level, typename... Args>
void log_if_enabled(const char *file, int line, const char *func, const char *message, Args &&...args) {
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

#define LOG_ERROR(...) log_if_enabled<LogLevel::ERROR>(__SHORT_FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_WARN(...)  log_if_enabled<LogLevel::WARN>(__SHORT_FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_INFO(...)  log_if_enabled<LogLevel::INFO>(__SHORT_FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_DEBUG(...) log_if_enabled<LogLevel::DEBUG>(__SHORT_FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_TRACE(...) log_if_enabled<LogLevel::TRACE>(__SHORT_FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

constexpr int HEADER_LENGTH = 40;
inline std::string FormatLogHeader(const char *file, int line, const char *func) {
	std::ostringstream ss;
	ss << file << ':' << line << ':' << func;
	std::string fullStr = ss.str();
	// Adjust the length to 20 characters
	if (fullStr.length() > HEADER_LENGTH) {
		// Truncate if longer than 20 characters
		return fullStr.substr(0, HEADER_LENGTH);
	} else {
		// Right pad with spaces if shorter than 20 characters
		return fullStr + std::string(HEADER_LENGTH - fullStr.length(), ' ');
	}
}

// Output log message header in this format: [type] [file:line:function] time -
// ex: [ERROR] [somefile.cpp:123:doSome()] 2008/07/06 10:00:00 -
inline void OutputLogHeader(const char *file, int line, const char *func, LogLevel level) {
	time_t t = time(nullptr);
	tm *curTime = localtime(&t); // NOLINT
	char time_str[32];
	strftime(time_str, 32, LOG_LOG_TIME_FORMAT, curTime);
	const char *type;
	switch (level) {
	case LogLevel::ERROR:
		type = "ERROR";
		break;
	case LogLevel::WARN:
		type = "WARN ";
		break;
	case LogLevel::INFO:
		type = "INFO ";
		break;
	case LogLevel::DEBUG:
		type = "DEBUG";
		break;
	case LogLevel::TRACE:
		type = "TRACE";
		break;
	default:
		type = "UNKWN";
	}
	std::string formattedHeader = FormatLogHeader(file, line, func);
	fprintf(LOG_OUTPUT_STREAM, "%s [%s] %s - ", time_str, formattedHeader.c_str(), type);
}

} // namespace db
