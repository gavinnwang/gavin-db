#include "buffer/buffer_pool_manager.hpp"
#include "common/typedef.hpp"
#include "storage/table/table_iterator.hpp"
#include "storage/table/table_meta.hpp"
#include "storage/table/tuple.hpp"

#include <memory>
#include <optional>
namespace db {

class TableHeap {
	friend class TableIterator;
	// in memory representation of table heap
public:
	DISALLOW_COPY_AND_MOVE(TableHeap);
	explicit TableHeap(const std::shared_ptr<BufferPoolManager> &bpm, const std::shared_ptr<TableMeta> &table_meta);
	std::optional<RID> InsertTuple(const TupleMeta &meta, const Tuple &tuple);
	void UpdateTupleMeta(const TupleMeta &meta, RID rid);
	std::optional<std::pair<TupleMeta, Tuple>> GetTuple(RID rid);
	TupleMeta GetTupleMeta(RID rid);
	page_id_t GetFirstPageId() const;
	TableIterator MakeIterator();

private:
	const std::shared_ptr<BufferPoolManager> &bpm_;
	const std::shared_ptr<TableMeta> &table_meta_;
	std::mutex latch_;
};
} // namespace db
