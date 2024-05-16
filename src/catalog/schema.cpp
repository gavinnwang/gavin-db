
#include "catalog/schema.hpp"
#include "catalog/column.hpp"
#include "common/macros.hpp"
#include <cstdint>
namespace db {
Schema::Schema(const std::vector<Column> &columns) {
  uint32_t curr_offset = 0;
  columns_.reserve(columns.size());
  for (uint32_t index = 0; index < columns.size(); index++) {
    Column column = columns[index];
    if (!column.IsInlined()) {
      uninlined_columns_.push_back(index);
    }
    column.column_offset_ = curr_offset;
    if (column.IsInlined()) {
      curr_offset += column.GetStorageSize();
    } else {
      // if not inlined we will store a offset pointer that will point to the
      // varlen at the end of tuple
      curr_offset += sizeof(uint32_t);
    }
    ASSERT(column.length_ > 0, "Invalid column length");
    columns_.push_back(column);
  }
  ASSERT(columns.size() == columns_.size(), "Column count mismatch");
  tuple_inline_part_storage_size_ = curr_offset;
}

void Schema::SerializeTo(char *storage) const {
  uint32_t sz = columns_.size();
  memcpy(storage, &sz, sizeof(uint32_t));
  uint32_t offset = sizeof(uint32_t);
  for (const auto &col : columns_) {
    col.SerializeTo(storage + offset);
    offset += col.GetSerializationSize();
  }
}

void Schema::DeserializeFrom(const char *storage) {
  uint32_t size = *reinterpret_cast<const uint32_t *>(storage);
  this->columns_.resize(size, Column());
  uint32_t offset = sizeof(uint32_t);
  for (uint32_t i = 0; i < size; i++) {
    columns_[i].DeserializeFrom(storage + offset);
    offset += columns_[i].GetSerializationSize();
  }
}

auto Schema::GetSerializationSize() const -> uint32_t {
  uint32_t size = sizeof(uint32_t);
  for (const auto &col : columns_) {
    size += col.GetSerializationSize();
  }
  return size;
}
} // namespace db
