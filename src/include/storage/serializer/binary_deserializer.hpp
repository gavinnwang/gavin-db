#pragma once

#include "storage/serializer/deserializer.hpp"
#include "storage/serializer/read_stream.hpp"

#include <cassert>
namespace db {

class BinaryDeserializer : public Deserializer {

private:
	ReadStream &stream;
	idx_t nesting_level = 0;

	bool has_buffered_field = false;
	field_id_t buffered_field = 0;

public:
	explicit BinaryDeserializer(ReadStream &stream) : stream(stream) {
	}

	template <class T>
	std::unique_ptr<T> Deserialize() {
		OnObjectBegin();
		auto result = T::Deserialize(*this);
		OnObjectEnd();
		assert(nesting_level == 0); // make sure we are at the root level
		return result;
	}

	template <class T>
	static std::unique_ptr<T> Deserialize(ReadStream &stream) {
		BinaryDeserializer deserializer(stream);
		return deserializer.Deserialize<T>();
	}

	ReadStream &GetStream() {
		return stream;
	}

private:
	field_id_t PeekField() {
		if (!has_buffered_field) {
			buffered_field = ReadPrimitive<field_id_t>();
			has_buffered_field = true;
		}
		return buffered_field;
	}
	void ConsumeField() {
		if (!has_buffered_field) {
			buffered_field = ReadPrimitive<field_id_t>();
		} else {
			has_buffered_field = false;
		}
	}
	field_id_t NextField() {
		if (has_buffered_field) {
			has_buffered_field = false;
			return buffered_field;
		}
		return ReadPrimitive<field_id_t>();
	}

	void ReadData(data_ptr_t buffer, idx_t read_size) {
		stream.ReadData(buffer, read_size);
	}

	template <class T>
	T ReadPrimitive() {
		T value;
		ReadData(data_ptr_cast(&value), sizeof(T));
		return value;
	}

	//===--------------------------------------------------------------------===//
	// Nested Types Hooks
	//===--------------------------------------------------------------------===//
	void OnPropertyBegin(const field_id_t field_id, const char *tag) final;
	void OnPropertyEnd() final;
	bool OnOptionalPropertyBegin(const field_id_t field_id, const char *tag) final;
	void OnOptionalPropertyEnd(bool present) final;
	void OnObjectBegin() final;
	void OnObjectEnd() final;
	idx_t OnListBegin() final;
	void OnListEnd() final;
	bool OnNullableBegin() final;
	void OnNullableEnd() final;

	//===--------------------------------------------------------------------===//
	// Primitive Types
	//===--------------------------------------------------------------------===//
	bool ReadBool() final;
	char ReadChar() final;
	int8_t ReadSignedInt8() final;
	uint8_t ReadUnsignedInt8() final;
	int16_t ReadSignedInt16() final;
	uint16_t ReadUnsignedInt16() final;
	int32_t ReadSignedInt32() final;
	uint32_t ReadUnsignedInt32() final;
	int64_t ReadSignedInt64() final;
	uint64_t ReadUnsignedInt64() final;
	float ReadFloat() final;
	double ReadDouble() final;
	std::string ReadString() final;
	// hugeint_t ReadHugeInt() final;
	// uhugeint_t ReadUhugeInt() final;
	void ReadDataPtr(data_ptr_t &ptr, idx_t count) final;
};
} // namespace db
