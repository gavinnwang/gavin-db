#pragma once
#include <iostream>
#include <stdexcept>
namespace db {

class Exception : public std::runtime_error {
public:
	explicit Exception(const std::string &message, bool print = true) : std::runtime_error(message) {
		if (print) {
			std::string exception_message = "Message :: " + message + "\n";
			std::cerr << exception_message;
		}
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

} // namespace db
