#include "storage/page/table_page.hpp"

#include "common/logger.hpp"
#include "common/macros.hpp"

#include <exception>

namespace db {
void TablePage::Init() {
	next_page_id_ = INVALID_PAGE_ID;
	num_tuples_ = 0;
	num_deleted_tuples_ = 0;
}
std::optional<uint16_t> TablePage::GetNextTupleOffset(const Tuple &tuple) const {
	size_t slot_end_offset;
	if (num_tuples_ > 0) {
		auto &[offset, size, meta] = tuple_info_[num_tuples_ - 1];
		slot_end_offset = offset;
	} else {
		slot_end_offset = PAGE_SIZE;
	}
	ENSURE(slot_end_offset >= TABLE_PAGE_HEADER_SIZE, "invalid slot end offset");
	ENSURE(tuple.GetStorageSize() <= PAGE_SIZE - TABLE_PAGE_HEADER_SIZE, "tuple is too large");

	// if the tuple size is already too large we return early to avoid underflow
	if (tuple.GetStorageSize() > slot_end_offset) {
		return std::nullopt;
	}
	auto tuple_offset = slot_end_offset - tuple.GetStorageSize();

	auto offset_size = TABLE_PAGE_HEADER_SIZE + TUPLE_INFO_SIZE * (num_tuples_ + 1);
	if (tuple_offset < offset_size) {
		return std::nullopt;
	}
	ENSURE(tuple_offset >= TABLE_PAGE_HEADER_SIZE && tuple_offset < PAGE_SIZE, "invalid tuple offset");
	return tuple_offset;
}

auto TablePage::InsertTuple(const TupleMeta &meta, const Tuple &tuple) -> std::optional<uint16_t> {
	auto tuple_offset = GetNextTupleOffset(tuple);
	if (tuple_offset == std::nullopt) {
		return std::nullopt;
	}
	auto tuple_id = num_tuples_;
	tuple_info_[tuple_id] = std::make_tuple(*tuple_offset, tuple.GetStorageSize(), meta);
	num_tuples_++;
	LOG_TRACE("offset={}", *tuple_offset);
	ASSERT(*tuple_offset + tuple.GetStorageSize() <= PAGE_SIZE, "tuple out of range");
	tuple.SerializeTo(page_start_ + *tuple_offset);
	return tuple_id;
}

void TablePage::UpdateTupleMeta(const TupleMeta &meta, const RID &rid) {
	auto tuple_id = rid.GetSlotNum();
	if (tuple_id >= num_tuples_) {
		throw Exception("Tuple ID out of range");
	}
	auto &[offset, size, old_meta] = tuple_info_[tuple_id];
	if (!old_meta.is_deleted_ && meta.is_deleted_) {
		num_deleted_tuples_++;
	}
	tuple_info_[tuple_id] = std::make_tuple(offset, size, meta);
}

auto TablePage::GetTuple(const RID &rid) const -> std::optional<std::pair<TupleMeta, Tuple>> {
	auto tuple_id = rid.GetSlotNum();
	if (tuple_id >= num_tuples_) {
		// return std::nullopt;
		throw Exception(fmt::format("Tuple ID out of range, {} >= {}", tuple_id, num_tuples_));
	}
	auto &[offset, size, meta] = tuple_info_[tuple_id];
	Tuple tuple;
	tuple.data_.resize(size);
	ASSERT(offset + size <= PAGE_SIZE, "tuple out of range");
	tuple.DeserializeFrom(page_start_ + offset, size);
	// memmove(tuple.data_.data(), page_start_ + offset, size);
	tuple.rid_ = rid;
	// printData(page_start_, PAGE_SIZE);
	return std::make_pair(meta, std::move(tuple));
}

auto TablePage::GetTupleMeta(const RID &rid) const -> TupleMeta {
	auto tuple_id = rid.GetSlotNum();
	if (tuple_id >= num_tuples_) {
		throw Exception("Tuple ID out of range");
	}
	auto &[_1, _2, meta] = tuple_info_[tuple_id];
	return meta;
}

} // namespace db
//
