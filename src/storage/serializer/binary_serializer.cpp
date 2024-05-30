#include "storage/serializer/binary_serializer.hpp"

#include "common/macros.hpp"
namespace db {

void BinarySerializer::OnPropertyBegin(const field_id_t field_id, const char *tag) {
	// Just write the field id straight up
	ASSERT(field_id != MESSAGE_TERMINATOR_FIELD_ID, "Field id cannot be MESSAGE_TERMINATOR_FIELD_ID");
	Write<field_id_t>(field_id);
	(void)tag;
}
void BinarySerializer::OnPropertyEnd() {
	// Nothing to do here
}

void BinarySerializer::OnOptionalPropertyBegin(const field_id_t field_id, const char *tag, bool present) {
	// Dont write anything at all if the property is not present
	if (present) {
		OnPropertyBegin(field_id, tag);
	}
}

void BinarySerializer::OnOptionalPropertyEnd(bool present) {
	(void)present;
	// Nothing to do here
}

void BinarySerializer::OnObjectBegin() {
	// #ifdef DEBUG
	// 	debug_stack.emplace_back();
	// #endif
}

void BinarySerializer::OnObjectEnd() {
	// #ifdef DEBUG
	// 	debug_stack.pop_back();
	// #endif
	// Write object terminator
	Write<field_id_t>(MESSAGE_TERMINATOR_FIELD_ID);
}

void BinarySerializer::OnListBegin(idx_t count) {
	Write<idx_t>(count);
	// Write(count);
}

void BinarySerializer::OnListEnd() {
}

void BinarySerializer::OnNullableBegin(bool present) {
	WriteValue(present);
}

void BinarySerializer::OnNullableEnd() {
}

void BinarySerializer::WriteValue(bool value) {
	Write(value);
}

void BinarySerializer::WriteValue(char value) {
	Write(value);
}

// template <typename T>
// typename std::enable_if<std::is_arithmetic<T>::value>::type BinarySerializer::WriteValue(T value) {
// 	Write(value);
// }

void BinarySerializer::WriteValue(uint8_t value) {
	Write(value);
}

void BinarySerializer::WriteValue(int8_t value) {
	Write(value);
}

void BinarySerializer::WriteValue(uint16_t value) {
	Write(value);
}

void BinarySerializer::WriteValue(int16_t value) {
	Write(value);
}

void BinarySerializer::WriteValue(uint32_t value) {
	Write(value);
}

void BinarySerializer::WriteValue(int32_t value) {
	Write(value);
}

void BinarySerializer::WriteValue(uint64_t value) {
	Write(value);
}

void BinarySerializer::WriteValue(int64_t value) {
	Write(value);
}

void BinarySerializer::WriteValue(float value) {
	Write(value);
}

void BinarySerializer::WriteValue(double value) {
	Write(value);
}

void BinarySerializer::WriteValue(const std::string &value) {
	auto len = static_cast<uint32_t>(value.length());
	Write(len);
	WriteData(value.c_str(), len);
}

void BinarySerializer::WriteValue(const char *value) {
	auto len = static_cast<uint32_t>(strlen(value));
	Write(len);
	WriteData(value, len);
}

void BinarySerializer::WriteDataPtr(const_data_ptr_t ptr, idx_t count) {
	Write(static_cast<uint64_t>(count));
	WriteData(ptr, count);
}

} // namespace db
