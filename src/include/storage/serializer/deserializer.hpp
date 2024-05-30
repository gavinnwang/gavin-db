#pragma once

#include "common/exception.hpp"
#include "common/typedef.hpp"
#include "storage/serializer/serialization_traits.hpp"
namespace db {

class Deserializer {

public:
	virtual ~Deserializer() {
	}
	// Read into an existing value
	template <typename T>
	inline void ReadProperty(const field_id_t field_id, const char *tag, T &ret) {
		OnPropertyBegin(field_id, tag);
		ret = Read<T>();
		OnPropertyEnd();
	}

	// Read and return a value
	template <typename T>
	inline T ReadProperty(const field_id_t field_id, const char *tag) {
		OnPropertyBegin(field_id, tag);
		auto ret = Read<T>();
		OnPropertyEnd();
		return ret;
	}

	// Default Value return
	template <typename T>
	inline T ReadPropertyWithDefault(const field_id_t field_id, const char *tag, T &&default_value) {
		if (!OnOptionalPropertyBegin(field_id, tag)) {
			OnOptionalPropertyEnd(false);
			return std::forward<T>(default_value);
		}
		auto ret = Read<T>();
		OnOptionalPropertyEnd(true);
		return ret;
	}

	// Default value in place
	template <typename T>
	inline void ReadPropertyWithDefault(const field_id_t field_id, const char *tag, T &ret, T &&default_value) {
		if (!OnOptionalPropertyBegin(field_id, tag)) {
			ret = std::forward<T>(default_value);
			OnOptionalPropertyEnd(false);
			return;
		}
		ret = Read<T>();
		OnOptionalPropertyEnd(true);
	}

	// Special case:
	// Read into an existing data_ptr_t
	inline void ReadProperty(const field_id_t field_id, const char *tag, data_ptr_t ret, idx_t count) {
		OnPropertyBegin(field_id, tag);
		ReadDataPtr(ret, count);
		OnPropertyEnd();
	}

private:
	// Deserialize anything implementing a Deserialize method
	template <typename T>
	inline T Read()
	    requires HasDeserialize<T>
	{
		OnObjectBegin();
		auto val = T::Deserialize(*this);
		OnObjectEnd();
		return val;
	}

	// Deserialize unique_ptr if the element type has a Deserialize method
	template <typename T>
	    requires IsUniquePtr<T> && HasDeserialize<typename T::element_type>
	T Read() {
		using ELEMENT_TYPE = typename T::element_type;
		std::unique_ptr<ELEMENT_TYPE> ptr = nullptr;
		auto is_present = OnNullableBegin();
		if (is_present) {
			OnObjectBegin();
			ptr = ELEMENT_TYPE::Deserialize(*this);
			OnObjectEnd();
		}
		OnNullableEnd();
		return ptr;
	}

	// Deserialize a unique_ptr if the element type does not have a Deserialize method
	template <typename T>
	    requires IsUniquePtr<T> && (!HasDeserialize<typename T::element_type>)
	T Read() {
		using ELEMENT_TYPE = typename T::element_type;
		std::unique_ptr<ELEMENT_TYPE> ptr = nullptr;
		auto is_present = OnNullableBegin();
		if (is_present) {
			OnObjectBegin();
			ptr = std::make_unique<ELEMENT_TYPE>(Read<ELEMENT_TYPE>());
			OnObjectEnd();
		}
		OnNullableEnd();
		return ptr;
	}

	// Deserialize shared_ptr
	template <typename T>
	    requires IsSharedPtr<T> && HasDeserialize<typename T::element_type>
	T Read() {
		using ELEMENT_TYPE = typename T::element_type;
		std::shared_ptr<ELEMENT_TYPE> ptr = nullptr;
		auto is_present = OnNullableBegin();
		if (is_present) {
			OnObjectBegin();
			ptr = ELEMENT_TYPE::Deserialize(*this);
			OnObjectEnd();
		}
		OnNullableEnd();
		return ptr;
	}

	template <typename T>
	inline T Read()
	    requires IsVector<T>
	{
		using ELEMENT_TYPE = typename T::value_type;
		T vec;
		auto size = OnListBegin();
		for (idx_t i = 0; i < size; i++) {
			vec.push_back(Read<ELEMENT_TYPE>());
		}
		OnListEnd();
		return vec;
	}

	template <typename T>
	inline T Read()
	    requires IsMap<T>
	{
		using KEY_TYPE = typename T::key_type;
		using VALUE_TYPE = typename T::mapped_type;
		T map;
		auto size = OnListBegin();
		for (idx_t i = 0; i < size; i++) {
			OnObjectBegin();
			auto key = ReadProperty<KEY_TYPE>(0, "key");
			auto value = ReadProperty<VALUE_TYPE>(1, "value");
			OnObjectEnd();
			map[std::move(key)] = std::move(value);
		}
		OnListEnd();
		return map;
	}

	template <typename T>
	inline T Read()
	    requires std::is_same_v<T, bool>
	{
		return ReadBool();
	}

	template <typename T>
	inline T Read()
	    requires std::is_same_v<T, char>
	{
		return ReadChar();
	}

	template <typename T>
	inline T Read()
	    requires std::is_same_v<T, int8_t>
	{
		return ReadSignedInt8();
	}

	template <typename T>
	inline T Read()
	    requires std::is_same_v<T, uint8_t>
	{
		return ReadUnsignedInt8();
	}

	template <typename T>
	inline T Read()
	    requires std::is_same_v<T, int16_t>
	{
		return ReadSignedInt16();
	}

	template <typename T>
	inline T Read()
	    requires std::is_same_v<T, uint16_t>
	{
		return ReadUnsignedInt16();
	}

	template <typename T>
	inline T Read()
	    requires std::is_same_v<T, int32_t>
	{
		return ReadSignedInt32();
	}

	template <typename T>
	inline T Read()
	    requires std::is_same_v<T, uint32_t>
	{
		return ReadUnsignedInt32();
	}

	template <typename T>
	inline T Read()
	    requires std::is_same_v<T, int64_t>
	{
		return ReadSignedInt64();
	}

	template <typename T>
	inline T Read()
	    requires std::is_same_v<T, uint64_t>
	{
		return ReadUnsignedInt64();
	}

	template <typename T>
	inline T Read()
	    requires std::is_same_v<T, float>
	{
		return ReadFloat();
	}

	template <typename T>
	inline T Read()
	    requires std::is_same_v<T, double>
	{
		return ReadDouble();
	}

	template <typename T>
	inline T Read()
	    requires std::is_same_v<T, std::string>
	{
		return ReadString();
	}

	template <typename T>
	inline T Read()
	    requires std::is_enum_v<T>
	{
		return static_cast<T>(Read<typename std::underlying_type<T>::type>());
	}

protected:
	// Hooks for subclasses to override to implement custom behavior
	virtual void OnPropertyBegin(const field_id_t field_id, const char *tag) = 0;
	virtual void OnPropertyEnd() = 0;
	virtual bool OnOptionalPropertyBegin(const field_id_t field_id, const char *tag) = 0;
	virtual void OnOptionalPropertyEnd(bool present) = 0;

	virtual void OnObjectBegin() = 0;
	virtual void OnObjectEnd() = 0;
	virtual idx_t OnListBegin() = 0;
	virtual void OnListEnd() = 0;
	virtual bool OnNullableBegin() = 0;
	virtual void OnNullableEnd() = 0;

	virtual bool ReadBool() = 0;
	virtual char ReadChar() {
		throw Exception("ReadChar not implemented for this deserializer");
	}
	virtual int8_t ReadSignedInt8() = 0;
	virtual uint8_t ReadUnsignedInt8() = 0;
	virtual int16_t ReadSignedInt16() = 0;
	virtual uint16_t ReadUnsignedInt16() = 0;
	virtual int32_t ReadSignedInt32() = 0;
	virtual uint32_t ReadUnsignedInt32() = 0;
	virtual int64_t ReadSignedInt64() = 0;
	virtual uint64_t ReadUnsignedInt64() = 0;
	virtual float ReadFloat() = 0;
	virtual double ReadDouble() = 0;
	virtual std::string ReadString() = 0;
	virtual void ReadDataPtr(data_ptr_t &ptr, idx_t count) = 0;
};
} // namespace db
