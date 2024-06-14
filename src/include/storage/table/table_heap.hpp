#include "buffer/buffer_pool_manager.hpp"
#include "common/typedef.hpp"
#include "storage/table/table_iterator.hpp"
#include "storage/table/table_meta.hpp"
#include "storage/table/tuple.hpp"

#include <memory>
#include <optional>
namespace db {

class TableHeap : public PageAllocator {
	friend class TableIterator;
	// in memory representation of table heap
public:
	DISALLOW_COPY_AND_MOVE(TableHeap);
	explicit TableHeap(const std::shared_ptr<BufferPoolManager> &bpm, const std::shared_ptr<TableMeta> &table_meta);
	std::optional<RID> InsertTuple(const TupleMeta &meta, const Tuple &tuple);
	void UpdateTupleMeta(const TupleMeta &meta, RID rid);
	std::optional<std::pair<TupleMeta, Tuple>> GetTuple(RID rid);
	TupleMeta GetTupleMeta(RID rid);
	[[nodiscard]] page_id_t GetFirstPageId() const;
	TableIterator MakeIterator();
	PageId AllocatePage() final {
		assert(table_meta_->table_oid_ != INVALID_TABLE_OID);
		table_meta_->last_table_heap_data_page_id_ = table_meta_->IncrementTableDataPageId();
		return {table_meta_->table_oid_, table_meta_->last_table_data_page_id_};
	}

private:
	const std::shared_ptr<BufferPoolManager> &bpm_;
	const std::shared_ptr<TableMeta> &table_meta_;
	std::mutex latch_;
};
} // namespace db
