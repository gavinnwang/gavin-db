#include "execution/executors/insert_executor.hpp"

#include "storage/table/table_heap.hpp"

namespace db {

bool InsertExecutor::Next(Tuple &tuple, RID &rid) {
	if (executed_) {
		return false;
	}

	int32_t changed_row_count = 0;
	Tuple t;
	RID r;

	auto &table_meta = exec_ctx_.GetCatalog().GetTable(plan_->GetTableOid());
	auto table_heap = std::make_unique<TableHeap>(exec_ctx_.GetBufferPoolManager(), table_meta);
	LOG_TRACE("created table heap");

	while (child_executor_->Next(t, r)) {
		TupleMeta meta = TupleMeta();
		meta.is_deleted_ = false;

		LOG_TRACE("got tuple {} from child executor", t.ToString(child_executor_->GetOutputSchema()));

		auto return_rid = table_heap->InsertTuple(meta, t);

		if (return_rid.has_value()) {
			rid = return_rid.value();
			changed_row_count++;
			table_meta.tuple_count_++;
			// TODO: insert to index

			// for (auto index : index_info_) {
			// 	auto key = t.KeyFromTuple(table_info_->schema_, index->key_schema_, index->index_->GetKeyAttrs());
			// 	auto rid_val = return_rid.value();
			// 	index->index_->InsertEntry(key, rid_val, exec_ctx_->GetTransaction());
			// }
		} else {
			throw std::runtime_error("Failed to insert tuple");
			return false;
		}
	}

	std::vector<Value> values = {Value(TypeId::INTEGER, changed_row_count)};
	tuple = Tuple(values, plan_->OutputSchema());

	executed_ = true;
	return true;
}

} // namespace db
