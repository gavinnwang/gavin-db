#include "storage/table_heap.hpp"
#include "common/macros.hpp"
#include "storage/table_page.hpp"
namespace db {

TableHeap::TableHeap(BufferPoolManager *bpm) {
  auto guard = bpm->NewPageGuarded(&first_page_id_);
  last_page_id_ = first_page_id_;
  auto first_page = guard.AsMut<TablePage>();
  ASSERT(first_page != nullptr, "table heap create page failed");
  first_page->Init();
};
auto TableHeap::InsertTuple(const TupleMeta &meta,
                            const Tuple &tuple) -> std::optional<RID> {

};
void TableHeap::UpdateTupleMeta(const TupleMeta &meta, RID rid) {

};
auto TableHeap::GetTuple(RID rid) -> std::pair<TupleMeta, Tuple> {

};
auto TableHeap::GetTupleMeta(RID rid) -> TupleMeta {

};
} // namespace db
