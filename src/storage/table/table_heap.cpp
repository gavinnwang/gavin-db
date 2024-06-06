
#include "storage/table/table_heap.hpp"

#include "buffer/buffer_pool_manager.hpp"
#include "common/macros.hpp"
#include "common/page_id.hpp"
#include "storage/page/table_page.hpp"
#include "storage/table/table_info.hpp"
#include "storage/table/table_iterator.hpp"

#include <memory>
namespace db {
TableHeap::TableHeap(std::shared_ptr<BufferPoolManager> bpm, std::shared_ptr<TableInfo> table_info)
    : bpm_(bpm), table_info_(table_info) {
	assert(table_info_ != nullptr);
	if (table_info_->GetLastTablePageId() == INVALID_PAGE_ID) {
		// page_id_t new_page_id;
		PageId new_page_id {table_info_->table_oid_};
		auto guard = bpm->NewPageGuarded(table_info_, new_page_id);
		ASSERT(new_page_id.page_number_ != INVALID_PAGE_ID && new_page_id.page_number_ >= 0,
		       "table heap create page failed");

		auto first_page = guard.AsMut<TablePage>();
		first_page->Init();
		// set the first and last page id to new page
		// table_info_->SetFirstTablePageId(new_page_id.page_number_);
		// table_info_->SetLastTablePageId(new_page_id.page_number_);
	}
	ASSERT(table_info_->GetLastTablePageId() != INVALID_PAGE_ID && table_info_->GetLastTablePageId() >= 0,
	       "table heap last page is invalid");
};

std::optional<RID> TableHeap::InsertTuple(const TupleMeta &meta, const Tuple &tuple) {
	std::unique_lock<std::mutex> guard(latch_);
	PageId new_page_id {table_info_->table_oid_, table_info_->GetLastTablePageId()};
	auto page_guard = bpm_->FetchPageWrite(new_page_id);
	while (true) {
		auto page = page_guard.AsMut<TablePage>();
		if (page->GetNextTupleOffset(tuple) != std::nullopt) {
			break;
		}
		// tuple page is empty and still cannot insert
		ENSURE(page->GetNumTuples() != 0, "tuple is too large");
		// allocate a new page for the tuple because the current page is full
		// page_id_t next_page_id = INVALID_PAGE_ID;
		PageId next_page_id {table_info_->table_oid_};
		auto npg = bpm_->NewPageGuarded(table_info_, next_page_id);
		ENSURE(next_page_id.page_number_ != INVALID_PAGE_ID, "cannot allocate page");
		// construct the linked list
		page->SetNextPageId(next_page_id.page_number_);
		// initialize the next page
		auto next_page = npg.AsMut<TablePage>();
		next_page->Init();
		// drop the current page
		page_guard.Drop();
		// fetch the next page
		auto next_page_guard = npg.UpgradeWrite();
		page_guard = std::move(next_page_guard);
	}
	auto last_page_id = table_info_->GetLastTablePageId();

	auto page = page_guard.AsMut<TablePage>();
	auto slot_id = *page->InsertTuple(meta, tuple);
	guard.unlock();
	page_guard.Drop();

	table_info_->IncreaseTupleCount();
	return RID({table_info_->table_oid_, last_page_id}, slot_id);
};

void TableHeap::UpdateTupleMeta(const TupleMeta &meta, RID rid) {
	auto page_guard = bpm_->FetchPageWrite(rid.GetPageId());
	auto page = page_guard.AsMut<TablePage>();
	page->UpdateTupleMeta(meta, rid);
};

std::optional<std::pair<TupleMeta, Tuple>> TableHeap::GetTuple(RID rid) {
	auto page_guard = bpm_->FetchPageRead(rid.GetPageId());
	auto page = page_guard.As<TablePage>();
	auto ret = page->GetTuple(rid);
	if (!ret.has_value()) {
		return std::nullopt;
	}
	auto [meta, tuple] = *ret;
	tuple.rid_ = rid;
	return std::make_pair(meta, std::move(tuple));
};

TupleMeta TableHeap::GetTupleMeta(RID rid) {

	auto page_guard = bpm_->FetchPageRead(rid.GetPageId());
	auto page = page_guard.As<TablePage>();
	return page->GetTupleMeta(rid);
};

TableIterator TableHeap::MakeIterator() {
	std::unique_lock<std::mutex> guard(latch_);
	auto table_oid = table_info_->table_oid_;
	auto last_page_id = table_info_->GetLastTablePageId();
	guard.unlock();

	auto page_guard = bpm_->FetchPageRead({table_oid, last_page_id});

	auto page = page_guard.As<TablePage>();
	auto num_tuples = page->GetNumTuples();
	page_guard.Drop();
	// iterate from rid 0, 0 to last_page_id and num_tuples
	return {this, {{table_oid, 0}, 0}, {{table_oid, last_page_id}, num_tuples}};
}

// auto TableHeap::GetFirstPageId() const -> page_id_t {
// 	auto page_guard = bpm_->FetchPageRead(table_info_id_);
// 	auto table_info = page_guard.As<TableInfoPage>();
// 	return table_info->GetFirstTablePageId();
// }

} // namespace db
