#include "storage/tuple.hpp"
#include "common/exception.hpp"
#include <cassert>
#include <sstream>
namespace db {

Tuple::Tuple(std::vector<Value> values, const Schema &schema) {
  assert(values.size() == schema.GetColumnCount());

  // 1. calc the size of tuple: inline storage size + variable storage size
  uint32_t tuple_size = schema.GetTupleInlinePartStorageSize();
  ASSERT(tuple_size > 0, "Tuple size must be greater than 0");
  ASSERT(tuple_size < PAGE_SIZE, "Tuple size must be less than PAGE_SIZE");
  for (auto &i : schema.GetUnlinedColumns()) {
    auto len = values[i].GetStorageSize();
    // if the length of the value is greater than the schema max length for that
    // column, error
    if (len > schema.GetColumn(i).GetStorageSize()) {
      throw Exception("Value length exceeds schema max length for column " +
                      schema.GetColumn(i).GetName() + " (max: " +
                      std::to_string(schema.GetColumn(i).GetStorageSize()) +
                      ", actual: " + std::to_string(len) + ")");
    }
    // size of uint32_t is the size info
    tuple_size += sizeof(uint32_t) + len;
  }
  data_.resize(tuple_size);

  std::fill(data_.begin(), data_.end(), 0);

  uint32_t column_count = schema.GetColumnCount();
  // uint32_t offset = schema.GetSerializationSize();
  uint32_t offset = schema.GetTupleInlinePartStorageSize();

  for (uint32_t i = 0; i < column_count; i++) {
    const auto &col = schema.GetColumn(i);
    if (!col.IsInlined()) {
      // serialize the relative offset
      *reinterpret_cast<uint32_t *>(data_.data() + col.GetOffset()) = offset;
      // serialize actual varchar value, in place (size | data)
      values[i].SerializeTo(data_.data() + offset);
    } else {
      values[i].SerializeTo(data_.data() + col.GetOffset());
    }
  }
}

// void Tuple::SerializeTo(char *storage) const {
//   int32_t size = data_.size();
//   memcpy(storage, &size, sizeof(int32_t));
//   memcpy(storage + sizeof(int32_t), data_.data(), size);
// }
//
// void Tuple::DeserializeFrom(const char *storage) {
//   uint32_t size = *reinterpret_cast<const uint32_t *>(storage);
//   this->data_.resize(size);
//   memcpy(this->data_.data(), storage + sizeof(int32_t), size);
// }

auto Tuple::GetValue(const Schema &schema, uint32_t column_idx) const -> Value {
  const TypeId column_type = schema.GetColumn(column_idx).GetType();
  const char *data_ptr = GetDataPtr(schema, column_idx);
  // the third parameter "is_inlined" is unused
  return Value::DeserializeFrom(data_ptr, column_type);
}

auto Tuple::GetDataPtr(const Schema &schema, uint32_t column_idx) const -> const
    char * {
  // assert(schema);
  const auto &col = schema.GetColumn(column_idx);
  bool is_inlined = col.IsInlined();
  if (is_inlined) {
    ASSERT(col.GetOffset() < data_.size(), "offset out of range");
    return (data_.data() + col.GetOffset());
  }

  // read the relative offset from the tuple data.
  int32_t offset =
      *reinterpret_cast<const int32_t *>(data_.data() + col.GetOffset());
  // return the beginning address of the real data for the VARCHAR type.
  return (data_.data() + offset);
}

auto Tuple::ToString(const Schema &schema) const -> std::string {
  std::stringstream os;

  int column_count = schema.GetColumnCount();
  bool first = true;
  os << "(";
  for (int column_itr = 0; column_itr < column_count; column_itr++) {
    if (first) {
      first = false;
    } else {
      os << ", ";
    }
    Value val = (GetValue(schema, column_itr));
    os << val.ToString();
  }
  os << ")";

  return os.str();
}
} // namespace db
