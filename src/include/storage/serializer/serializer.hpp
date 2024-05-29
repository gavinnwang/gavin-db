#pragma once

#include "common/config.hpp"
#include "common/exception.hpp"
#include "common/value.hpp"
#include "storage/serializer/serialization_traits.hpp"

#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

namespace db {
class Serializer {
protected:
	// bool serialize_enum_as_string = false;
	bool serialize_default_values = false;

public:
	// Serialize a value
	template <class T>
	void WriteProperty(const field_id_t field_id, const char *tag, const T &value) {
		OnPropertyBegin(field_id, tag);
		WriteValue(value);
		OnPropertyEnd();
	}

	template <class T>
	void WritePropertyWithDefault(const field_id_t field_id, const char *tag, const T &value, const T &&default_value) {
		// If current value is default, don't write it
		if (!serialize_default_values && (value == default_value)) {
			OnOptionalPropertyBegin(field_id, tag, false);
			OnOptionalPropertyEnd(false);
			return;
		}
		OnOptionalPropertyBegin(field_id, tag, true);
		WriteValue(value);
		OnOptionalPropertyEnd(true);
	}
	//
	// Specialization for Value (default Value comparison throws when comparing
	// nulls)
	template <>
	void WritePropertyWithDefault<Value>(const field_id_t field_id, const char *tag, const Value &value,
	                                     const Value &&default_value) {
		// If current value is default, don't write it
		// TODO
		throw Exception("Not implemented");
	}

protected:
	template <typename T>
	typename std::enable_if<std::is_enum<T>::value, void>::type WriteValue(const T value) {
		// if (serialize_enum_as_string) {
		// 	// Use the enum serializer to lookup tostring function
		// 	auto str = EnumUtil::ToChars(value);
		// 	WriteValue(str);
		// } else {
			// Use the underlying type
			WriteValue(static_cast<std::underlying_type<T>::type>(value));
		// }
	}
	// Unique Pointer Ref
	template <typename T>
	void WriteValue(const std::unique_ptr<T> &ptr) {
		WriteValue(ptr.get());
	}

	// Shared Pointer Ref
	template <typename T>
	void WriteValue(const std::shared_ptr<T> &ptr) {
		WriteValue(ptr.get());
	}

	// Pointer
	template <typename T>
	void WriteValue(const T *ptr) {
		if (ptr == nullptr) {
			OnNullableBegin(false);
			OnNullableEnd();
		} else {
			OnNullableBegin(true);
			WriteValue(*ptr);
			OnNullableEnd();
		}
	}

	template <class K, class V>
	void WriteValue(const std::pair<K, V> &pair) {
		// OnObjectBegin();
		WriteProperty(0, "first", pair.first);
		WriteProperty(1, "second", pair.second);
		// OnObjectEnd();
	}

	// Vector
	template <class T>
	void WriteValue(const std::vector<T> &vec) {
		auto count = vec.size();
		OnListBegin(count);
		for (auto &item : vec) {
			WriteValue(item);
		}
		OnListEnd();
	}

	// Set
	// Serialized the same way as a list/vector
	template <class T, class HASH, class CMP>
	void WriteValue(const std::set<T, HASH, CMP> &set) {
		auto count = set.size();
		OnListBegin(count);
		for (auto &item : set) {
			WriteValue(item);
		}
		OnListEnd();
	}

	// Map
	// serialized as a list of pairs
	template <class K, class V, class HASH, class CMP>
	void WriteValue(const std::unordered_map<K, V, HASH, CMP> &map) {
		auto count = map.size();
		OnListBegin(count);
		for (auto &item : map) {
			OnObjectBegin();
			WriteProperty(0, "key", item.first);
			WriteProperty(1, "value", item.second);
			OnObjectEnd();
		}
		OnListEnd();
	}

	// class or struct implementing `Serialize(Serializer& Serializer)`;
	template <typename T>
	typename std::enable_if<has_serialize<T>::value>::type WriteValue(const T &value) {
		OnObjectBegin();
		value.Serialize(*this);
		OnObjectEnd();
	}

protected:
	// Hooks for subclasses to override to implement custom behavior
	virtual void OnPropertyBegin(const field_id_t field_id, const char *tag) = 0;
	virtual void OnPropertyEnd() = 0;
	virtual void OnOptionalPropertyBegin(const field_id_t field_id, const char *tag, bool present) = 0;
	virtual void OnOptionalPropertyEnd(bool present) = 0;
	virtual void OnObjectBegin() = 0;
	virtual void OnObjectEnd() = 0;
	virtual void OnListBegin(idx_t count) = 0;
	virtual void OnListEnd() = 0;
	virtual void OnNullableBegin(bool present) = 0;
	virtual void OnNullableEnd() = 0;

	// Handle primitive types, a serializer needs to implement these.
	virtual void WriteNull() = delete;
	virtual void WriteValue(char value) {
		throw Exception("Write char value not implemented");
		// throw NotImplementedException("Write char value not implemented");
	}
	virtual void WriteValue(bool value) = 0;

	// virtual template <typename T>
	// typename std::enable_if<std::is_arithmetic<T>::value>::type WriteValue(T value) = 0;
	virtual void WriteValue(uint8_t value) = 0;
	virtual void WriteValue(int8_t value) = 0;
	virtual void WriteValue(uint16_t value) = 0;
	virtual void WriteValue(int16_t value) = 0;
	virtual void WriteValue(uint32_t value) = 0;
	virtual void WriteValue(int32_t value) = 0;
	virtual void WriteValue(uint64_t value) = 0;
	virtual void WriteValue(int64_t value) = 0;
	virtual void WriteValue(float value) = 0;
	virtual void WriteValue(double value) = 0;
	virtual void WriteValue(const std::string &value) = 0;
	virtual void WriteValue(const char *str) = 0;
	virtual void WriteDataPtr(const_data_ptr_t ptr, idx_t count) = 0;
};
} // namespace db
