#include "common/value.hpp"

#include "common/macros.hpp"
#include "common/type.hpp"

#include <cstdint>
#include <cstring>
#include <variant>
namespace db {

void Value::Serialize(Serializer &serializer) const {

	serializer.WriteProperty(100, "type", type_id_);
	serializer.WriteProperty(101, "is_null", is_null_);

	if (!IsNull()) {
		switch (type_id_) {
		case TypeId::INVALID:
			UNREACHABLE("Cannot serialize invalid type value");
		case TypeId::BOOLEAN:
			serializer.WriteProperty(102, "value", std::get<int8_t>(value_));
			break;
		case TypeId::INTEGER:
			serializer.WriteProperty(102, "value", std::get<int32_t>(value_));
			break;
		case TypeId::TIMESTAMP:
			serializer.WriteProperty(102, "value", std::get<uint64_t>(value_));
			break;
		case TypeId::VARCHAR:
			serializer.WriteProperty(102, "value", std::get<std::string>(value_));
			break;
		default:
			throw NotImplementedException("Unimplemented type for Serialize");
		}
	}
}

Value Value::Deserialize(Deserializer &deserializer) {
	auto type_id = deserializer.ReadProperty<TypeId>(100, "type");
	auto is_null = deserializer.ReadProperty<bool>(101, "is_null");
	Value new_value = Value(type_id);
	if (is_null) {
		return new_value;
	}
	new_value.is_null_ = false;
	switch (type_id) {
	case TypeId::INVALID:
		UNREACHABLE("Cannot serialize invalid type value");
	case TypeId::BOOLEAN:
		new_value.value_ = deserializer.ReadProperty<int8_t>(102, "value");
		break;
	case TypeId::INTEGER:
		new_value.value_ = deserializer.ReadProperty<int32_t>(102, "value");
		break;
	case TypeId::TIMESTAMP:
		new_value.value_ = deserializer.ReadProperty<uint64_t>(102, "value");
		break;
	case TypeId::VARCHAR:
		new_value.value_ = deserializer.ReadProperty<std::string>(102, "value");
		break;
	default:
		throw NotImplementedException("Unimplemented type for Serialize");
	}

	return new_value;
};

void Value::SerializeTo(data_ptr_t storage) const {
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
