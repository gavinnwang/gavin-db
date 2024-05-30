#include "common/value.hpp"

#include "common/macros.hpp"
#include "common/type.hpp"

#include <cstdint>
#include <cstring>
#include <variant>
namespace db {

void Value::SerializeTo(char *storage) const {
	switch (type_id_) {
	case TypeId::BOOLEAN: {
		ASSERT(std::holds_alternative<int8_t>(value_), "Invalid variant type for BOOLEAN Value");
		*reinterpret_cast<int8_t *>(storage) = std::get<int8_t>(value_);
		return;
	}
	case TypeId::INTEGER: {
		*reinterpret_cast<int32_t *>(storage) = std::get<int32_t>(value_);
		return;
	}
	case TypeId::TIMESTAMP: {
		*reinterpret_cast<uint64_t *>(storage) = std::get<uint64_t>(value_);
		return;
	}
	case TypeId::VARCHAR: {
		auto &varchar_val = std::get<std::string>(value_);
		uint32_t var_len = varchar_val.size();
		memcpy(storage, &var_len, sizeof(uint32_t));
		memcpy(storage + sizeof(uint32_t), varchar_val.data(), var_len);
		return;
	}
	case TypeId::INVALID: {
		UNREACHABLE("Cannot serialize invalid type valye");
		return;
	}
	}
}

auto Value::ToString() const -> std::string {
	switch (type_id_) {
	case TypeId::BOOLEAN:
		return std::to_string(std::get<int8_t>(value_));
	case TypeId::INTEGER:
		return std::to_string(std::get<int32_t>(value_));
	case TypeId::TIMESTAMP:
		return std::to_string(std::get<uint64_t>(value_));
	case TypeId::VARCHAR:
		return std::get<std::string>(value_);
	case TypeId::INVALID:
		UNREACHABLE("Cannot convert invalid type value to string");
		return "";
	}
}
} // namespace db
