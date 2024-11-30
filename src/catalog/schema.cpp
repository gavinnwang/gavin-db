
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
		column.storage_offset_ = curr_offset;
		column.schema_offset_ = index;
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

} // namespace db
