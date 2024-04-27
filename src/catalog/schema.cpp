
#include "catalog/schema.hpp"
#include "catalog/column.hpp"
namespace db {
Schema::Schema(const std::vector<Column> &columns) : columns_(columns) {
  uint32_t curr_offset = 0;
  columns_.reserve(columns_.size());
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
    columns_[index] = column;
  }
  inline_storage_size_ = curr_offset;
}
} // namespace db
