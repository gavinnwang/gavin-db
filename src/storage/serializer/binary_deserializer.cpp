#include "storage/serializer/binary_deserializer.hpp"

#include "common/exception.hpp"
#include "common/logger.hpp"

#include <memory>
#include <variant>
namespace db {
using std::string;

void BinaryDeserializer::OnPropertyBegin(field_id_t field_id, const char *tag) {
	(void)tag;
	auto field = NextField();
	if (field != field_id) {
		LOG_ERROR("Failed to deserialize: field id mismatch, expected: %d, but got: %d", field_id, field);
		throw SerializationException("Failed to deserialize: field id mismatch, expected: " + std::to_string(field_id) +
		                             ", but got: " + std::to_string(field));
	}
}

void BinaryDeserializer::OnPropertyEnd() {
}

bool BinaryDeserializer::OnOptionalPropertyBegin(field_id_t field_id, const char *tag) {
	(void)tag;
	auto next_field = PeekField();
	auto present = next_field == field_id;
	if (present) {
		ConsumeField();
	}

	return present;
}

void BinaryDeserializer::OnOptionalPropertyEnd(bool present) {
	(void)present;
}

void BinaryDeserializer::OnObjectBegin() {
	nesting_level_++;
}

void BinaryDeserializer::OnObjectEnd() {
	auto next_field = NextField();
	if (next_field != MESSAGE_TERMINATOR_FIELD_ID) {
		throw SerializationException("Failed to deserialize: expected end of object, but found field id: " +
		                             std::to_string(next_field));
	}
	nesting_level_--;
}

idx_t BinaryDeserializer::OnListBegin() {
	return ReadPrimitive<idx_t>();
}

void BinaryDeserializer::OnListEnd() {
}

bool BinaryDeserializer::OnNullableBegin() {
	return ReadBool();
}

void BinaryDeserializer::OnNullableEnd() {
}

//-------------------------------------------------------------------------
// Primitive Types
//-------------------------------------------------------------------------
bool BinaryDeserializer::ReadBool() {
	return static_cast<bool>(ReadPrimitive<uint8_t>());
}

char BinaryDeserializer::ReadChar() {
	return ReadPrimitive<char>();
}

int8_t BinaryDeserializer::ReadSignedInt8() {
	return ReadPrimitive<int8_t>();
}

uint8_t BinaryDeserializer::ReadUnsignedInt8() {
	return ReadPrimitive<uint8_t>();
}

int16_t BinaryDeserializer::ReadSignedInt16() {
	return ReadPrimitive<int16_t>();
}

uint16_t BinaryDeserializer::ReadUnsignedInt16() {
	return ReadPrimitive<uint16_t>();
}

int32_t BinaryDeserializer::ReadSignedInt32() {
	return ReadPrimitive<int32_t>();
}

uint32_t BinaryDeserializer::ReadUnsignedInt32() {
	return ReadPrimitive<uint32_t>();
}

int64_t BinaryDeserializer::ReadSignedInt64() {
	return ReadPrimitive<int64_t>();
}

uint64_t BinaryDeserializer::ReadUnsignedInt64() {
	return ReadPrimitive<uint64_t>();
}

float BinaryDeserializer::ReadFloat() {
	auto value = ReadPrimitive<float>();
	return value;
}

double BinaryDeserializer::ReadDouble() {
	auto value = ReadPrimitive<double>();
	return value;
}

std::string BinaryDeserializer::ReadString() {
	auto len = ReadPrimitive<uint32_t>();
	if (len == 0) {
		return {};
	}

	std::vector<data_t> buffer(len);
	ReadData(buffer.data(), len);
	return {const_char_ptr_cast(buffer.data()), len};
}

void BinaryDeserializer::ReadDataPtr(data_ptr_t &ptr_p, idx_t count) {
	auto len = ReadPrimitive<uint64_t>();
	if (len != count) {
		throw SerializationException("Tried to read blob of " + std::to_string(count) + " size, but only " +
		                             std::to_string(len) + " elements are available");
	}
	ReadData(ptr_p, count);
}

} // namespace db
