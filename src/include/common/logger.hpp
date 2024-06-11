#pragma once

#include <cstdio>
#include <ctime>

namespace db {

// https://blog.galowicz.de/2016/02/20/short_file_macro/
// Log levelsusing cstr = const char * const;
using cstr = const char *const;

static constexpr cstr past_last_slash(cstr str, cstr last_slash) {
	return *str == '\0'  ? last_slash
	       : *str == '/' ? past_last_slash(str + 1, str + 1)
	                     : past_last_slash(str + 1, last_slash);
}

static constexpr cstr past_last_slash(cstr str) {
	return past_last_slash(str, str);
}
#define __SHORT_FILE__                                                                                                 \
	({                                                                                                                 \
		constexpr cstr sf__ {past_last_slash(__FILE__)};                                                               \
		sf__;                                                                                                          \
	})

static constexpr int LOG_LEVEL_OFF = 1000;
static constexpr int LOG_LEVEL_ERROR = 500;
static constexpr int LOG_LEVEL_WARN = 400;
static constexpr int LOG_LEVEL_INFO = 300;
static constexpr int LOG_LEVEL_DEBUG = 200;
static constexpr int LOG_LEVEL_TRACE = 100;
static constexpr int LOG_LEVEL_ALL = 0;

// #define LOG_LOG_TIME_FORMAT "%Y-%m-%d %H:%M:%S"
#define LOG_LOG_TIME_FORMAT "%M:%S"
#define LOG_OUTPUT_STREAM   stdout

static constexpr int LOG_LEVEL = LOG_LEVEL_ALL;

// For compilers which do not support __FUNCTION__
#if !defined(__FUNCTION__) && !defined(__GNUC__)
#define __FUNCTION__ ""
#endif

void OutputLogHeader(const char *file, int line, const char *func, int level);

#if LOG_LEVEL <= LOG_LEVEL_ERROR
#define LOG_ERROR_ENABLED
#define LOG_ERROR(...)                                                                                                 \
	OutputLogHeader(__SHORT_FILE__, __LINE__, __FUNCTION__, LOG_LEVEL_ERROR);                                          \
	::fprintf(LOG_OUTPUT_STREAM, __VA_ARGS__);                                                                         \
	fprintf(LOG_OUTPUT_STREAM, "\n");                                                                                  \
	::fflush(stdout)
#else
#define LOG_ERROR(...) ((void)0)
#endif

#ifdef LOG_WARN_ENABLED
#undef LOG_WARN_ENABLED
#endif
#if LOG_LEVEL <= LOG_LEVEL_WARN
#define LOG_WARN_ENABLED
#define LOG_WARN(...)                                                                                                  \
	OutputLogHeader(__SHORT_FILE__, __LINE__, __FUNCTION__, LOG_LEVEL_WARN);                                           \
	::fprintf(LOG_OUTPUT_STREAM, __VA_ARGS__);                                                                         \
	fprintf(LOG_OUTPUT_STREAM, "\n");                                                                                  \
	::fflush(stdout)
#else
#define LOG_WARN(...) ((void)0)
#endif

#ifdef LOG_INFO_ENABLED
#undef LOG_INFO_ENABLED
#endif
#if LOG_LEVEL <= LOG_LEVEL_INFO
#define LOG_INFO_ENABLED
#define LOG_INFO(...)                                                                                                  \
	OutputLogHeader(__SHORT_FILE__, __LINE__, __FUNCTION__, LOG_LEVEL_INFO);                                           \
	::fprintf(LOG_OUTPUT_STREAM, __VA_ARGS__);                                                                         \
	fprintf(LOG_OUTPUT_STREAM, "\n");                                                                                  \
	::fflush(stdout)
#else
#define LOG_INFO(...) ((void)0)
#endif

#ifdef LOG_DEBUG_ENABLED
#undef LOG_DEBUG_ENABLED
#endif
#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define LOG_DEBUG_ENABLED
#define LOG_DEBUG(...)                                                                                                 \
	OutputLogHeader(__SHORT_FILE__, __LINE__, __FUNCTION__, LOG_LEVEL_DEBUG);                                          \
	::fprintf(LOG_OUTPUT_STREAM, __VA_ARGS__);                                                                         \
	fprintf(LOG_OUTPUT_STREAM, "\n");                                                                                  \
	::fflush(stdout)
#else
#define LOG_DEBUG(...) ((void)0)
#endif

#ifdef LOG_TRACE_ENABLED
#undef LOG_TRACE_ENABLED
#endif
#if LOG_LEVEL <= LOG_LEVEL_TRACE
#define LOG_TRACE_ENABLED
#define LOG_TRACE(...)                                                                                                 \
	OutputLogHeader(__SHORT_FILE__, __LINE__, __FUNCTION__, LOG_LEVEL_TRACE);                                          \
	::fprintf(LOG_OUTPUT_STREAM, __VA_ARGS__);                                                                         \
	fprintf(LOG_OUTPUT_STREAM, "\n");                                                                                  \
	::fflush(stdout)
#else
#define LOG_TRACE(...) ((void)0)
#endif

// Output log message header in this format: [type] [file:line:function] time -
// ex: [ERROR] [somefile.cpp:123:doSome()] 2008/07/06 10:00:00 -
inline void OutputLogHeader(const char *file, int line, const char *func, int level) {
	time_t t = ::time(nullptr);
	tm *curTime = localtime(&t); // NOLINT
	char time_str[32];           // FIXME
	::strftime(time_str, 32, LOG_LOG_TIME_FORMAT, curTime);
	const char *type;
	switch (level) {
	case LOG_LEVEL_ERROR:
		type = "ERROR";
		break;
	case LOG_LEVEL_WARN:
		type = "WARN ";
		break;
	case LOG_LEVEL_INFO:
		type = "INFO ";
		break;
	case LOG_LEVEL_DEBUG:
		type = "DEBUG";
		break;
	case LOG_LEVEL_TRACE:
		type = "TRACE";
		break;
	default:
		type = "UNKWN";
	}

	::fprintf(LOG_OUTPUT_STREAM, "%s [%s:%d:%s] %s - ", time_str, file, line, func, type);
}

} // namespace db
