#pragma once
#include "common/logger.hpp"

#include <stdexcept>
namespace db {

class Exception : public std::runtime_error {
public:
	explicit Exception(const std::string &message) : std::runtime_error(message) {
		LOG_ERROR(message);
	}
};

class NotImplementedException : public Exception {
public:
	explicit NotImplementedException(const std::string &msg) : Exception(msg) {
	}
};

class SerializationException : public Exception {
public:
	explicit SerializationException(const std::string &msg) : Exception(msg) {
	}
};

class IOException : public Exception {
public:
	explicit IOException(const std::string &msg) : Exception(msg) {
	}
};

class RuntimeException : public Exception {
public:
	explicit RuntimeException(const std::string &msg) : Exception(msg) {
	}
};

} // namespace db
