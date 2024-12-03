#pragma once

#include "execution/executors/abstract_executor.hpp"
#include "execution/plans/seq_scan_plan.hpp"
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

	const Schema &GetOutputSchema() const override {
		return plan_->OutputSchema();
	}

private:
	std::unique_ptr<SeqScanPlanNode> plan_;
	// TODO add table heap pool
	TableHeap table_heap_;
	TableIterator table_iter_;
};
} // namespace db