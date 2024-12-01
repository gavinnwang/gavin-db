#pragma once

#include "common/exception.hpp"
#include "common/macros.hpp"
#include "sql/ColumnType.h"

#include <cstdint>
#include <string>
#include <utility>
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
		std::unreachable();
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
		std::unreachable();
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
		std::unreachable();
	}

	static TypeId HsqlColumnTypeToTypeId(hsql::ColumnType col_type) {
		switch (col_type.data_type) {
		case hsql::DataType::BOOLEAN:
			return TypeId::BOOLEAN;
		case hsql::DataType::INT:
			return TypeId::INTEGER;
		case hsql::DataType::TIME:
			return TypeId::TIMESTAMP;
		case hsql::DataType::VARCHAR:
			return TypeId::VARCHAR;
		case hsql::DataType::UNKNOWN:
		case hsql::DataType::BIGINT:
		case hsql::DataType::CHAR:
		case hsql::DataType::DATE:
		case hsql::DataType::DATETIME:
		case hsql::DataType::DECIMAL:
		case hsql::DataType::DOUBLE:
		case hsql::DataType::FLOAT:
		case hsql::DataType::LONG:
		case hsql::DataType::REAL:
		case hsql::DataType::SMALLINT:
		case hsql::DataType::TEXT:
			throw NotImplementedException("Type is not implemented");
		}
		throw NotImplementedException("Type is not implemented");
	}
};
} // namespace db
