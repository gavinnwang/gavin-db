#pragma once

#include "common/macros.hpp"
#include "common/rid.hpp"
#include "storage/table/tuple.hpp"

#include <cassert>
#include <utility>

namespace db {

class TableHeap;

/**
 * TableIterator enables the sequential scan of a TableHeap.
 */
class TableIterator {
	friend class Cursor;

public:
	DISALLOW_COPY_AND_MOVE(TableIterator);

	TableIterator(const TableHeap &table_heap, RID rid, RID stop_at_rid)
	    : table_heap_(table_heap), rid_(rid), stop_at_rid_(stop_at_rid) {};

	~TableIterator() = default;

	std::optional<std::pair<TupleMeta, Tuple>> GetTuple();

	auto GetRID() -> RID;

	auto IsEnd() -> bool;

	auto operator++() -> TableIterator &;

private:
	const TableHeap &table_heap_;
	RID rid_;
	RID stop_at_rid_;
};

} // namespace db
