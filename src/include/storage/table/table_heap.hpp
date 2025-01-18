#include "storage/buffer/buffer_pool.hpp"
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
	TableHeap(const TableHeap &) = delete;
	TableHeap &operator=(const TableHeap &) = delete;
	TableHeap(TableHeap &&) = delete;
	TableHeap &operator=(TableHeap &&) = delete;
	explicit TableHeap(BufferPool &bpm, TableMeta &table_meta);
	// doesn't ensure the tuple is the same schema as the table
	[[nodiscard]] std::optional<RID> InsertTuple(const TupleMeta &meta, const Tuple &tuple);
	void UpdateTupleMeta(const TupleMeta &meta, RID rid);
	[[nodiscard]] std::optional<std::pair<TupleMeta, Tuple>> GetTuple(RID rid) const;
	[[nodiscard]] TupleMeta GetTupleMeta(RID rid);
	[[nodiscard]] page_id_t GetFirstPageId() const;
	[[nodiscard]] TableIterator MakeIterator();
	[[nodiscard]] PageId AllocatePage() final {
		assert(table_meta_.table_oid_ != INVALID_TABLE_OID);
		table_meta_.last_table_heap_data_page_id_ = table_meta_.IncrementTableDataPageId();
		return {table_meta_.table_oid_, table_meta_.last_table_data_page_id_};
	}

private:
	BufferPool &bpm_;
	TableMeta &table_meta_;
	std::mutex latch_;
};
} // namespace db
