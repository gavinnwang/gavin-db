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

} // namespace db
