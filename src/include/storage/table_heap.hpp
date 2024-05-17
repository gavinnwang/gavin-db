#include "buffer/buffer_pool_manager.hpp"
#include "storage/tuple.hpp"
#include <optional>
namespace db {
class TableHeap {
  // in memory representation of table heap
public:
  ~TableHeap() = default;
  explicit TableHeap(BufferPoolManager *bpm, page_id_t table_info_page_id);
  auto InsertTuple(const TupleMeta &meta,
                   const Tuple &tuple) -> std::optional<RID>;
  void UpdateTupleMeta(const TupleMeta &meta, RID rid);
  auto GetTuple(RID rid) -> std::pair<TupleMeta, Tuple>;
  auto GetTupleMeta(RID rid) -> TupleMeta;
  auto GetFirstPageId() const -> page_id_t;

private:
  BufferPoolManager *bpm_;
  page_id_t table_info_page_id_;
  // page_id_t first_page_id_{INVALID_PAGE_ID};
  std::mutex latch_;
  // page_id_t last_page_id_{INVALID_PAGE_ID}; /* protected by latch_ */
};
} // namespace db
