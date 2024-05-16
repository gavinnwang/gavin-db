#include "storage/table_heap.hpp"
#include "common/config.hpp"
#include "common/macros.hpp"
#include "storage/table_page.hpp"
namespace db {

TableHeap::TableHeap(BufferPoolManager *bpm) : bpm_(bpm) {
  auto guard = bpm->NewPageGuarded(&first_page_id_);
  last_page_id_ = first_page_id_;
  auto first_page = guard.AsMut<TablePage>();
  ASSERT(first_page != nullptr, "table heap create page failed");
  first_page->Init();
};
auto TableHeap::InsertTuple(const TupleMeta &meta,
                            const Tuple &tuple) -> std::optional<RID> {
  std::unique_lock<std::mutex> guard(latch_);
  auto page_guard = bpm_->FetchPageWrite(last_page_id_);
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
    last_page_id_ = next_page_id;
    page_guard = std::move(next_page_guard);
  }
  auto last_page_id = last_page_id_;

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
} // namespace db
