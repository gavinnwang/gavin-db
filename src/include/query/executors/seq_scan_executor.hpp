#pragma once

#include "query/executors/abstract_executor.hpp"
#include "query/plans/seq_scan_plan.hpp"
#include "storage/table/table_heap.hpp"
#include "storage/table/table_iterator.hpp"
namespace db {

/** The SeqScanExecutor executor executes a sequential table scan. */
class SeqScanExecutor : public AbstractExecutor {
public:
	SeqScanExecutor(const ExecutorContext &exec_context, std::unique_ptr<SeqScanPlanNode> plan)
	    : AbstractExecutor(exec_context), plan_(std::move(plan)),
	      table_heap_(
	          TableHeap(exec_context.GetBufferPoolManager(), exec_context.GetCatalog().GetTable(plan_->table_oid_))),
	      table_iter_(table_heap_.MakeIterator()) {
	}

	bool Next(Tuple &tuple, RID &rid) override;

	[[nodiscard]] const Schema &GetOutputSchema() const override {
		return plan_->OutputSchema();
	}

private:
	std::unique_ptr<SeqScanPlanNode> plan_;
	// TODO(gavinwang): add table heap pool
	TableHeap table_heap_;
	TableIterator table_iter_;
};
} // namespace db
