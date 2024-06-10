#pragma once

#include "common/config.hpp"
#include "common/exception.hpp"
#include "common/logger.hpp"
#include "common/macros.hpp"
#include "common/type.hpp"
#include "storage/serializer/deserializer.hpp"
#include "storage/serializer/serializer.hpp"

#include <cstdint>
#include <cstring>
#include <variant>

namespace db {
template <typename T>
bool ValueIsCorrectType(TypeId type) {
	switch (type) {
	case TypeId::BOOLEAN:
		return typeid(T) == typeid(uint8_t);
	case TypeId::INTEGER:
		return typeid(T) == typeid(int32_t);
	case TypeId::TIMESTAMP:
		return typeid(T) == typeid(uint64_t);
	case TypeId::VARCHAR:
		return typeid(T) == typeid(std::string);
	default:
		return false;
	}
}

class Value {
public:
	// create an value with type_id
	Value(TypeId type) : type_id_(type), is_null_(true) {
	}

	template <typename T>
	Value(TypeId type, T &&i) : type_id_(type), value_(std::forward<T>(i)), is_null_(false) {
		if (!ValueIsCorrectType<T>(type)) {
			LOG_ERROR("Value isn't assigned to the correct type id, expected %s, got %s",
			          Type::TypeIdToString(type).c_str(), typeid(T).name());
			throw RuntimeException("Value isn't assigned to the correct type id");
		}
	};

	// Get the length of the variable length data
	inline auto GetStorageSize() const -> uint32_t {
		return Type::TypeSize(type_id_, GetVarlenStorageSize());
	}

	inline auto IsNull() const -> bool {
		return is_null_;
	}

	void SerializeTo(data_ptr_t storage) const;
	auto ToString() const -> std::string;

	void Serialize(Serializer &serializer) const;
	static Value Deserialize(Deserializer &deserializer);

	static Value DeserializeFromWithTypeInfo(const_data_ptr_t storage) {
		auto type_id = *reinterpret_cast<const TypeId *>(storage);
		return DeserializeFrom(storage + sizeof(TypeId), type_id);
	}

	void SerializeToWithTypeInfo(data_ptr_t storage) const {
		*reinterpret_cast<TypeId *>(storage) = type_id_;
		SerializeTo(storage + sizeof(TypeId));
	}

	static auto DeserializeFrom(const_data_ptr_t storage, const TypeId type_id) -> Value {
		switch (type_id) {
		case TypeId::BOOLEAN: {
			int8_t val = *reinterpret_cast<const int8_t *>(storage);
			return {type_id, val};
		}
		case TypeId::INTEGER: {
			int32_t val = *reinterpret_cast<const int32_t *>(storage);
			return {type_id, val};
		}
		case TypeId::TIMESTAMP: {
			uint64_t val = *reinterpret_cast<const uint32_t *>(storage);
			return {type_id, val};
		}
		case TypeId::VARCHAR: {
			uint32_t var_len = *reinterpret_cast<const uint32_t *>(storage);
			ASSERT(var_len < PAGE_SIZE, "Invalid varchar length");
			return {type_id, std::string(reinterpret_cast<const char *>(storage) + sizeof(uint32_t), var_len)};
		}
		case TypeId::INVALID: {
			UNREACHABLE("Value has invalid type id");
		}
		}
	}

	const IndexKeyType ConvertToIndexKeyType() const {
		IndexKeyType ret = {0};
		switch (type_id_) {
		case TypeId::BOOLEAN: {
			memcpy(&ret, &value_, sizeof(int8_t));
			return ret;
		}
		case TypeId::INTEGER: {
			memcpy(&ret, &value_, sizeof(int32_t));
			return ret;
		}
		case TypeId::TIMESTAMP: {
			memcpy(&ret, &value_, sizeof(uint32_t));
			return ret;
		}
		case TypeId::VARCHAR: {
			memcpy(&ret, &value_, sizeof(IndexKeyType) - 1);
			size_t var_len = std::get<std::string>(value_).size();
			ret[std::min(var_len, sizeof(IndexKeyType) - 1)] = '\0';
			return ret;
		}
		case TypeId::INVALID: {
			UNREACHABLE("Value has invalid type id");
		}
		}
	}

private:
	uint32_t GetVarlenStorageSize() const {
		if (type_id_ != TypeId::VARCHAR) {
			return 0;
		} else {
			return std::get<std::string>(value_).size();
		}
	}
	TypeId type_id_;

	// Define the variant to hold all possible types
	using Val = std::variant<int8_t, int32_t, uint64_t, std::string>;

	Val value_;
	bool is_null_;
};
} // namespace db
