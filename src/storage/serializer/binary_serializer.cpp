#include "storage/serializer/binary_serializer.hpp"
namespace db {

void BinarySerializer::OnPropertyBegin(const field_id_t field_id,
                                       const char *tag) {
  // Just write the field id straight up
  Write<field_id_t>(field_id);
  (void)tag;
}
void BinarySerializer::OnPropertyEnd() {
  // Nothing to do here
}

void BinarySerializer::OnListBegin(idx_t count) { WriteVarInt(count); }

void BinarySerializer::OnListEnd() {}

void BinarySerializer::OnNullableBegin(bool present) { WriteValue(present); }

void BinarySerializer::OnNullableEnd() {}

void BinarySerializer::WriteValue(bool value) { Write<uint8_t>(value); }

void BinarySerializer::WriteValue(uint8_t value) { WriteVarInt(value); }

void BinarySerializer::WriteValue(char value) { Write(value); }

void BinarySerializer::WriteValue(int8_t value) { WriteVarInt(value); }

void BinarySerializer::WriteValue(uint16_t value) { WriteVarInt(value); }

void BinarySerializer::WriteValue(int16_t value) { WriteVarInt(value); }

void BinarySerializer::WriteValue(uint32_t value) { WriteVarInt(value); }

void BinarySerializer::WriteValue(int32_t value) { WriteVarInt(value); }

void BinarySerializer::WriteValue(uint64_t value) { WriteVarInt(value); }

void BinarySerializer::WriteValue(int64_t value) { WriteVarInt(value); }

void BinarySerializer::WriteValue(float value) { Write(value); }

void BinarySerializer::WriteValue(double value) { Write(value); }

void BinarySerializer::WriteValue(const std::string &value) {
  auto len = static_cast<uint32_t>(value.length());
  WriteVarInt(len);
  WriteData(value.c_str(), len);
}

void BinarySerializer::WriteValue(const char *value) {
  auto len = static_cast<uint32_t>(strlen(value));
  WriteVarInt(len);
  WriteData(value, len);
}

void BinarySerializer::WriteDataPtr(const_data_ptr_t ptr, idx_t count) {
  WriteVarInt(static_cast<uint64_t>(count));
  WriteData(ptr, count);
}

} // namespace db
