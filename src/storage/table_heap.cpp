#include "storage/table_heap.hpp"

#include "common/config.hpp"
#include "common/macros.hpp"
#include "storage/table_page.hpp"

#include <memory>
namespace db {

TableHeap::TableHeap(BufferPoolManager *bpm, std::shared_ptr<TableInfo> table_info)
    : bpm_(bpm), table_info_(std::move(table_info)) {
	assert(table_info_ != nullptr);
	if (table_info_->GetFirstTablePageId() == INVALID_PAGE_ID) {
		page_id_t new_page_id;
		auto guard = bpm->NewPageGuarded(new_page_id);
		std::cout << "new_page_id " << new_page_id;
		ASSERT(new_page_id != INVALID_PAGE_ID && new_page_id >= 0, "table heap create page failed");

		auto first_page = guard.AsMut<TablePage>();
		first_page->Init();
		// set the first and last page id to new page
		table_info_->SetFirstTablePageId(new_page_id);
		table_info_->SetLastTablePageId(new_page_id);
	}
	// std::cout << "TableHeap::TableHeap" << table_info_->GetFirstTablePageId() << std::endl;
	// std::cout << "TableHeap::TableHeap" << table_info_->GetLastTablePageId() << std::endl;
	ASSERT(table_info_->GetLastTablePageId() != INVALID_PAGE_ID &&
	           table_info_->GetFirstTablePageId() != INVALID_PAGE_ID && table_info_->GetLastTablePageId() >= 0 &&
	           table_info_->GetFirstTablePageId() >= 0,
	       "table heap last page is invalid");
};
auto TableHeap::InsertTuple(const TupleMeta &meta, const Tuple &tuple) -> std::optional<RID> {
	std::unique_lock<std::mutex> guard(latch_);

	// auto table_info_pg = bpm_->FetchPageWrite(table_info_id_);
	// auto table_info = table_info_pg.AsMut<TableInfoPage>();
	auto page_guard = bpm_->FetchPageWrite(table_info_->GetLastTablePageId());
	while (true) {
		auto page = page_guard.AsMut<TablePage>();
		if (page->GetNextTupleOffset(tuple) != std::nullopt) {
			break;
		}
		// tuple page is empty and still cannot insert
		ENSURE(page->GetNumTuples() != 0, "tuple is too large");

		// allocate a new page for the tuple because the current page is full
		page_id_t next_page_id = INVALID_PAGE_ID;
		auto npg = bpm_->NewPageGuarded(next_page_id);
		ENSURE(next_page_id != INVALID_PAGE_ID, "cannot allocate page");
		ENSURE(next_page_id != INVALID_PAGE_ID, "cannot allocate page");

		// construct the linked list
		page->SetNextPageId(next_page_id);

		// initialize the next page
		// auto next_page = reinterpret_cast<TablePage *>(npg->GetData());
		auto next_page = npg.AsMut<TablePage>();
		next_page->Init();

		// drop the current page
		page_guard.Drop();

		// fetch the next page
		auto next_page_guard = npg.UpgradeWrite();
		// update the last page id

		// last_page_id_ = next_page_id;
		table_info_->SetLastTablePageId(next_page_id);
		page_guard = std::move(next_page_guard);
	}
	auto last_page_id = table_info_->GetLastTablePageId();

	auto page = page_guard.AsMut<TablePage>();
	auto slot_id = *page->InsertTuple(meta, tuple);
	guard.unlock();

	page_guard.Drop();
	return RID(last_page_id, slot_id);
};

void TableHeap::UpdateTupleMeta(const TupleMeta &meta, RID rid) {
	auto page_guard = bpm_->FetchPageWrite(rid.GetPageId());
	auto page = page_guard.AsMut<TablePage>();
	page->UpdateTupleMeta(meta, rid);
};

auto TableHeap::GetTuple(RID rid) -> std::pair<TupleMeta, Tuple> {
	auto page_guard = bpm_->FetchPageRead(rid.GetPageId());
	auto page = page_guard.As<TablePage>();
	auto [meta, tuple] = page->GetTuple(rid);
	tuple.rid_ = rid;
	return std::make_pair(meta, std::move(tuple));
};

auto TableHeap::GetTupleMeta(RID rid) -> TupleMeta {

	auto page_guard = bpm_->FetchPageRead(rid.GetPageId());
	auto page = page_guard.As<TablePage>();
	return page->GetTupleMeta(rid);
};

// auto TableHeap::GetFirstPageId() const -> page_id_t {
// 	auto page_guard = bpm_->FetchPageRead(table_info_id_);
// 	auto table_info = page_guard.As<TableInfoPage>();
// 	return table_info->GetFirstTablePageId();
// }

} // namespace db
