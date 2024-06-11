#include "buffer/buffer_pool_manager.hpp"
#include "common/typedef.hpp"
#include "storage/table/table_meta.hpp"
#include "storage/table/table_iterator.hpp"
#include "storage/table/tuple.hpp"

#include <memory>
#include <optional>
namespace db {

class TableHeap {
	friend class TableIterator;
	// in memory representation of table heap
public:
	explicit TableHeap(std::shared_ptr<BufferPoolManager> bpm, std::shared_ptr<TableMeta> table_meta);
	std::optional<RID> InsertTuple(const TupleMeta &meta, const Tuple &tuple);
	void UpdateTupleMeta(const TupleMeta &meta, RID rid);
	std::optional<std::pair<TupleMeta, Tuple>> GetTuple(RID rid);
	TupleMeta GetTupleMeta(RID rid);
	page_id_t GetFirstPageId() const;
	TableIterator MakeIterator();

private:
	std::shared_ptr<BufferPoolManager> bpm_;
	std::shared_ptr<TableMeta> table_meta_;
	std::mutex latch_;
};
} // namespace db
