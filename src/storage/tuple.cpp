#include "storage/tuple.hpp"
#include <sstream>
namespace db {

Tuple::Tuple(std::vector<Value> values, const Schema *schema) {
  assert(values.size() == schema->GetColumnCount());

  // 1. calc the size of tuple: inline storage size + variable storage size
  uint32_t tuple_size = schema->GetTupleInlinePartStorageSize();
  for (auto &i : schema->GetUnlinedColumns()) {
    auto len = values[i].GetStorageSize();
    // size of uint32_t is the size info
    tuple_size += sizeof(uint32_t) + len;
  }
  data_.resize(tuple_size);

  std::fill(data_.begin(), data_.end(), 0);

  uint32_t column_count = schema->GetColumnCount();
  uint32_t offset = schema->GetSerializationSize();

  for (uint32_t i = 0; i < column_count; i++) {
    const auto &col = schema->GetColumn(i);
    if (!col.IsInlined()) {
      // serialize the relative offset
      *reinterpret_cast<uint32_t *>(data_.data() + col.GetOffset()) = offset;
      // serialize actual varchar value, in place (size | data)
      values[i].SerializeTo(data_.data() + offset);
    }
  }
}

void Tuple::SerializeTo(char *storage) const {
  int32_t size = data_.size();
  memcpy(storage, &size, sizeof(int32_t));
  memcpy(storage + sizeof(int32_t), data_.data(), size);
}

void Tuple::DeserializeFrom(const char *storage) {
  uint32_t size = *reinterpret_cast<const uint32_t *>(storage);
  this->data_.resize(size);
  memcpy(this->data_.data(), storage + sizeof(int32_t), size);
}

auto Tuple::GetValue(const Schema &schema, uint32_t column_idx) const -> Value {
  const TypeId column_type = schema.GetColumn(column_idx).GetType();
  const char *data_ptr = GetDataPtr(schema, column_idx);
  // the third parameter "is_inlined" is unused
  return Value::DeserializeFrom(data_ptr, column_type);
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
