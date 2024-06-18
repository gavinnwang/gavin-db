#pragma once

#include "common/macros.hpp"

#include <cstdint>
#include <string>
namespace db {
enum class TypeId : uint8_t { INVALID = 0, BOOLEAN, INTEGER, TIMESTAMP, VARCHAR };

class Type {
public:
	static bool IsFixedSize(TypeId type_id) {
		switch (type_id) {
		case TypeId::BOOLEAN:
		case TypeId::INTEGER:
		case TypeId::TIMESTAMP:
			return true;
		case TypeId::VARCHAR:
			return false;
		default: {
			UNREACHABLE("Cannot get size of invalid type");
		}
		}
	}
	static uint32_t TypeSize(TypeId type_id, uint32_t length = 0) {
		switch (type_id) {
		case TypeId::BOOLEAN:
			return 1;
		case TypeId::INTEGER:
			return 4;
		case TypeId::TIMESTAMP:
			return 8;
		case TypeId::VARCHAR:
			return length;
		default: {
			UNREACHABLE("Cannot get size of invalid type");
		}
		}
	}

	static std::string TypeIdToString(TypeId type_id) {
		switch (type_id) {
		case TypeId::BOOLEAN:
			return "BOOLEAN";
		case TypeId::INTEGER:
			return "INTEGER";
		case TypeId::TIMESTAMP:
			return "TIMESTAMP";
		case TypeId::VARCHAR:
			return "VARCHAR";
		default: {
			UNREACHABLE("Unkown type");
		}
		}
	}
};
} // namespace db
