#pragma once

#include <string>
#include <utility>
namespace db {
enum class ArithmeticType { Plus, Minus, Multiply };
class ArithmeticTypeHelper {
public:
	static std::string ToString(ArithmeticType type) {
		switch (type) {
		case ArithmeticType::Plus:
			return "+";
		case ArithmeticType::Minus:
			return "-";
		case ArithmeticType::Multiply:
			return "*";
		}
		std::unreachable();
	}
};

} // namespace db
