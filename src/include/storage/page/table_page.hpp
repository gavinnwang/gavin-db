#pragma once
#include "storage/table/tuple.hpp"

#include <cstdint>
#include <optional>
#include <tuple>
namespace db {
static constexpr uint64_t TABLE_PAGE_HEADER_SIZE = 8;
class TablePage {
public:
	TablePage() = delete;
	~TablePage() = delete;
	void Init();
	[[nodiscard]] auto GetNumTuples() const -> uint32_t {
		return num_tuples_;
	}
	[[nodiscard]] auto GetNextPageId() const -> page_id_t {
		return next_page_id_;
	}
	void SetNextPageId(page_id_t next_page_id) {
		next_page_id_ = next_page_id;
	}
	auto GetNextTupleOffset(const Tuple &tuple) const -> std::optional<uint16_t>;
	auto InsertTuple(const TupleMeta &meta, const Tuple &tuple) -> std::optional<uint16_t>;
	void UpdateTupleMeta(const TupleMeta &meta, const RID &rid);
	[[nodiscard]] auto GetTupleMeta(const RID &rid) const -> TupleMeta;
	[[nodiscard]] auto GetTuple(const RID &rid) const -> std::optional<std::pair<TupleMeta, Tuple>>;
	void UpdateTupleInPlaceUnsafe(const TupleMeta &meta, const Tuple &tuple, RID rid);

private:
	// slotted page
	// header format:  NextPageId (4)| NumTuples(2) | NumDeletedTuples(2)
	// tuple 1 offset+size+meta | tuple 2 offset+size+meta
	using TupleInfo = std::tuple<uint16_t, uint16_t, TupleMeta>;

	char page_start_[0];
	page_id_t next_page_id_;
	uint16_t num_tuples_;
	uint16_t num_deleted_tuples_;
	TupleInfo tuple_info_[0];

	static constexpr size_t TUPLE_INFO_SIZE = 6;
	static_assert(std::is_trivially_copyable_v<TupleMeta>);
	static_assert(std::is_trivially_copyable_v<uint16_t>);
	// static_assert(std::is_trivially_copyable_v<TupleInfo> == true);
	static_assert(sizeof(TupleInfo) == TUPLE_INFO_SIZE);
	static_assert(sizeof(page_id_t) == 4);
};
static_assert(sizeof(TablePage) == TABLE_PAGE_HEADER_SIZE);
} // namespace db
