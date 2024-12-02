#include "common/config.hpp"
#include "common/exception.hpp"
#include "storage/page/table_page.hpp"
#include "storage/table/table_heap.hpp"

#include <cassert>
#include <optional>

namespace db {

std::optional<std::pair<TupleMeta, Tuple>> TableIterator::GetTuple() {
	return table_heap_.GetTuple(rid_);
}

auto TableIterator::GetRID() -> RID {
	return rid_;
}

auto TableIterator::IsEnd() -> bool {
	return rid_.GetPageId().page_number_ == INVALID_PAGE_ID;
}

auto TableIterator::operator++() -> TableIterator & {
	auto page_guard = table_heap_.bpm_.FetchPageRead(rid_.GetPageId());
	const auto &page = page_guard.As<TablePage>();
	auto next_tuple_id = rid_.GetSlotNum() + 1;

	if (stop_at_rid_.GetPageId().page_number_ != INVALID_PAGE_ID) {
		ASSERT(
		    /* case 1: cursor before the page of the stop tuple */ rid_.GetPageId().page_number_ <
		            stop_at_rid_.GetPageId().page_number_ ||
		        /* case 2: cursor at the page before the tuple */
		        (rid_.GetPageId() == stop_at_rid_.GetPageId() && next_tuple_id <= stop_at_rid_.GetSlotNum()),
		    "iterate out of bound");
	}

	rid_ = RID {rid_.GetPageId(), next_tuple_id};

	if (rid_ == stop_at_rid_) {
		rid_ = RID {{table_heap_.table_meta_.table_oid_, INVALID_PAGE_ID}, 0};
	} else if (next_tuple_id < page.GetNumTuples()) {
		// that's fine
	} else {
		auto next_page_id = page.GetNextPageId();
		// if next page is invalid, RID is set to invalid page; otherwise, it's the first tuple in that page.
		rid_ = RID {{table_heap_.table_meta_.table_oid_, next_page_id}, 0};
	}

	page_guard.Drop();

	return *this;
}

} // namespace db
