#include "storage/tuple.hpp"
namespace db {

Tuple::Tuple(std::vector<Value> values, const Schema *schema) {
  assert(values.size() == schema->GetColumnCount());

  // 1. calc the size of tuple: inline storage size + variable storage size
  uint32_t tuple_size = schema->GetInlinedStorageSize();
  for (auto &i : schema->GetUnlinedColumns()) {
    auto len = values[i].GetStorageSize();
    // size of uint32_t is the size info
    tuple_size += sizeof(uint32_t) + len;
  }
  data_.resize(tuple_size);

  std::fill(data_.begin(), data_.end(), 0);

  uint32_t column_count = schema->GetColumnCount();
  uint32_t offset = schema->GetInlinedStorageSize();

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
} // namespace db
