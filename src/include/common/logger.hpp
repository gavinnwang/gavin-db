#pragma once

#include "fmt/core.h"

#include <array>
#include <ctime>
#include <string>

namespace db {

// Log levels
enum class LogLevel { OFF = 1000, ERROR = 500, WARN = 400, INFO = 300, DEBUG = 200, TRACE = 100, ALL = 0 };

// Define the current log level
constexpr LogLevel CURRENT_LOG_LEVEL = LogLevel::ALL;

// Utility to extract the file name
constexpr std::string_view PastLastSlash(std::string_view str, std::string_view last_slash) {
	return str.empty()          ? last_slash
	       : str.front() == '/' ? PastLastSlash(str.substr(1), str.substr(1))
	                            : PastLastSlash(str.substr(1), last_slash);
}
constexpr std::string_view PastLastSlash(std::string_view str) {
	return PastLastSlash(str, str);
}

#define __SHORT_FILE__                                                                                                 \
	([] {                                                                                                              \
		constexpr std::string_view sf__ {PastLastSlash(__FILE__)};                                                     \
		return sf__;                                                                                                   \
	}())

// Time format and output stream
#define LOG_LOG_TIME_FORMAT "%M:%S"
#define LOG_OUTPUT_STREAM   stdout

// For compilers which do not support __FUNCTION__
#if !defined(__FUNCTION__) && !defined(__GNUC__)
#define __FUNCTION__ ""
#endif

constexpr bool IsLogLevelEnabled(LogLevel level) {
	return level >= CURRENT_LOG_LEVEL;
}

constexpr int HEADER_LENGTH = 40;
inline std::string FormatLogHeader(std::string_view file, int line, const char *func) {
	std::string full_str = fmt::format("{}:{}:{}", file, line, func);
	if (full_str.length() > HEADER_LENGTH) {
		return full_str.substr(0, HEADER_LENGTH);
	}
	return full_str + std::string(HEADER_LENGTH - full_str.length(), ' ');
}

// Output log message header in this format: [type] [file:line:function] time -
// ex: [ERROR] [somefile.cpp:123:doSome()] 2008/07/06 10:00:00 -
[[nodiscard]] inline std::string LogHeader(std::string_view file, int line, const char *func, LogLevel level) {
	time_t t = time(nullptr);
	tm *cur_time = localtime(&t);
	std::array<char, 32> time_str;
	strftime(time_str.data(), time_str.size(), LOG_LOG_TIME_FORMAT, cur_time);
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
	std::string formatted_header = FormatLogHeader(file, line, func);
	return fmt::format("[{}] {} - ", time_str.data(), formatted_header + " " + type);
}

template <LogLevel level, typename... Args>
void Log(std::string_view file, int line, const char *func, const std::string &message, Args &&...args) {
	if constexpr (IsLogLevelEnabled(level)) {
		auto header = LogHeader(file, line, func, level);
		if constexpr (!sizeof...(Args)) {
			fmt::print(LOG_OUTPUT_STREAM, "{}\n", message);
		} else {
			if constexpr (std::is_same_v<std::decay_t<decltype(message)>, const char *>) {
				auto formatted_message = fmt::format(fmt::runtime(message), std::forward<Args>(args)...);
				fprintf(LOG_OUTPUT_STREAM, "%s\n", formatted_message.c_str());
			} else if constexpr (std::is_same_v<std::decay_t<decltype(message)>, std::string>) {
				auto formatted_message = fmt::format(fmt::runtime(message), std::forward<Args>(args)...);
				fmt::print(LOG_OUTPUT_STREAM, "{}\n", formatted_message);
			}
		}

		fflush(LOG_OUTPUT_STREAM); // Ensure log is flushed
	}
}

#define LOG_ERROR(...) Log<LogLevel::ERROR>(__SHORT_FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_WARN(...)  Log<LogLevel::WARN>(__SHORT_FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_INFO(...)  Log<LogLevel::INFO>(__SHORT_FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_DEBUG(...) Log<LogLevel::DEBUG>(__SHORT_FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_TRACE(...) Log<LogLevel::TRACE>(__SHORT_FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

} // namespace db
