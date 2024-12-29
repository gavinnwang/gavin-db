#pragma once

#include "storage/serializer/serializer.hpp"
#include "storage/serializer/write_stream.hpp"

namespace db {
class BinarySerializer : public Serializer {

public:
	explicit BinarySerializer(WriteStream &stream, bool serialize_default_values_p = false) : stream_(stream) {
		serialize_default_values_ = serialize_default_values_p;
	}

private:
	void WriteData(const_data_ptr_t buffer, idx_t write_size) {
		stream_.WriteData(buffer, write_size);
	}

	void WriteData(const char *ptr, idx_t write_size) {
		WriteData(const_data_ptr_cast(ptr), write_size);
	}

	template <class T>
	void Write(T element) {
		static_assert(std::is_trivially_destructible<T>(), "Write element must be trivially destructible");
		WriteData(const_data_ptr_cast(&element), sizeof(T));
	}

	// template <class T>
	// void WriteVarInt(T value) {
	// 	// uint8_t buffer[16];
	// 	// uint32_t write_size = sizeof(value);
	// 	// // first encode the size of value and followed by the value
	// 	//
	// 	// // auto write_size = EncodingUtil::EncodeLEB128<T>(buffer, value);
	// 	// ASSERT(write_size <= sizeof(buffer), "VarIntEncode buffer overflow");
	// 	// WriteData(buffer, write_size);
	// }

public:
	template <class T>
	static void Serialize(const T &value, WriteStream &stream, bool serialize_default_values = false) {
		BinarySerializer serializer(stream, serialize_default_values);
		serializer.OnObjectBegin();
		value.Serialize(serializer);
		serializer.OnObjectEnd();
	}

protected:
	//-------------------------------------------------------------------------
	// Nested Type Hooks
	//-------------------------------------------------------------------------
	// We serialize optional values as a message with a "present" flag, followed
	// by the value.
	void OnPropertyBegin(field_id_t field_id, const char *tag) final;
	void OnPropertyEnd() final;
	void OnOptionalPropertyBegin(field_id_t field_id, const char *tag, bool present) final;
	void OnOptionalPropertyEnd(bool present) final;
	void OnListBegin(idx_t count) final;
	void OnListEnd() final;
	void OnObjectBegin() final;
	void OnObjectEnd() final;
	void OnNullableBegin(bool present) final;
	void OnNullableEnd() final;

	//-------------------------------------------------------------------------
	// Primitive Types
	//-------------------------------------------------------------------------
	void WriteValue(char value) final;
	// template <typename T>
	// typename std::enable_if<std::is_arithmetic<T>::value>::type WriteValue(T value);
	void WriteValue(uint8_t value) final;
	void WriteValue(int8_t value) final;
	void WriteValue(uint16_t value) final;
	void WriteValue(int16_t value) final;
	void WriteValue(uint32_t value) final;
	void WriteValue(int32_t value) final;
	void WriteValue(uint64_t value) final;
	void WriteValue(int64_t value) final;
	void WriteValue(float value) final;
	void WriteValue(double value) final;
	// void WriteValue(const string_t value) final;
	void WriteValue(const std::string &value) final;
	void WriteValue(const char *value) final;
	void WriteValue(bool value) final;
	void WriteDataPtr(const_data_ptr_t ptr, idx_t count) final;

private:
	WriteStream &stream_;

public:
	void WriteNull() final = delete;
};
} // namespace db
