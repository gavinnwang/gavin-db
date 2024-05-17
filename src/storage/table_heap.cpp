#include "storage/table_heap.hpp"
#include "common/config.hpp"
#include "common/macros.hpp"
#include "storage/table_info_page.hpp"
#include "storage/table_page.hpp"
namespace db {

TableHeap::TableHeap(BufferPoolManager *bpm, page_id_t table_info_page_id)
    : bpm_(bpm), table_info_page_id_(table_info_page_id) {
  auto table_info_pg = bpm_->FetchPageWrite(table_info_page_id_);
  auto table_info_page = table_info_pg.AsMut<TableInfoPage>();

  ASSERT(table_info_page->GetTableInfo().name_.size() > 0,
         "table name is empty");

  if (table_info_page->GetFirstTablePageId() == INVALID_PAGE_ID) {
    page_id_t new_page_id;
    auto guard = bpm->NewPageGuarded(&new_page_id);
    ASSERT(new_page_id != INVALID_PAGE_ID, "table heap create page failed");
    auto first_page = guard.AsMut<TablePage>();
    first_page->Init();
    // set the first and last page id to new page
    table_info_page->SetFirstTablePageId(new_page_id);
    table_info_page->SetLastTablePageId(new_page_id);
  }
  ASSERT(table_info_page->GetLastTablePageId() != INVALID_PAGE_ID &&
             table_info_page->GetFirstTablePageId() != INVALID_PAGE_ID &&
             table_info_page->GetLastTablePageId() > 0 &&
             table_info_page->GetFirstTablePageId() > 0,
         "table heap last page is invalid");
};
auto TableHeap::InsertTuple(const TupleMeta &meta,
                            const Tuple &tuple) -> std::optional<RID> {
  std::unique_lock<std::mutex> guard(latch_);

  auto table_info_pg = bpm_->FetchPageWrite(table_info_page_id_);
  auto table_info_page = table_info_pg.AsMut<TableInfoPage>();

  auto page_guard =
      bpm_->FetchPageWrite(table_info_page->GetFirstTablePageId());
  while (true) {
    auto page = page_guard.AsMut<TablePage>();
    if (page->GetNextTupleOffset(tuple) != std::nullopt) {
      break;
    }
    // tuple page is empty and still cannot insert
    ENSURE(page->GetNumTuples() != 0, "tuple is too large");

    // allocate a new page for the tuple because the current page is full
    page_id_t next_page_id = INVALID_PAGE_ID;
    auto npg = bpm_->NewPage(&next_page_id);
    ENSURE(next_page_id != INVALID_PAGE_ID, "cannot allocate page");

    // construct the linked list
    page->SetNextPageId(next_page_id);

    // initialize the next page
    auto next_page = reinterpret_cast<TablePage *>(npg->GetData());
    next_page->Init();

    // drop the current page
    page_guard.Drop();

    // fetch the next page
    auto next_page_guard = WritePageGuard{bpm_, npg};

    // update the last page id

    // last_page_id_ = next_page_id;
    table_info_page->SetLastTablePageId(next_page_id);
    page_guard = std::move(next_page_guard);
  }
  auto last_page_id = table_info_page->GetLastTablePageId();

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

auto TableHeap::GetFirstPageId() const -> page_id_t {
  auto page_guard = bpm_->FetchPageRead(table_info_page_id_);
  auto table_info_page = page_guard.As<TableInfoPage>();
  return table_info_page->GetFirstTablePageId();
}

} // namespace db
