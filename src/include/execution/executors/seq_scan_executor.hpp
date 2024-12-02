#pragma once

#include "execution/executors/abstract_executor.hpp"
#include "execution/plans/seq_scan_plan.hpp"
#include "storage/table/table_heap.hpp"
namespace db {

/** The SeqScanExecutor executor executes a sequential table scan. */
class SeqScanExecutor : public AbstractExecutor {
public:
	SeqScanExecutor(const ExecutorContext &exec_context, std::unique_ptr<SeqScanPlanNode> plan)
	    : AbstractExecutor(exec_context), plan_(std::move(plan)) {
		auto &table_meta = exec_context.GetCatalog().GetTable(plan_->table_oid_);
		auto &bpm = exec_context.GetBufferPoolManager();
		auto table_heap = std::make_unique<TableHeap>(bpm, table_meta);
	}

	bool Next(Tuple &tuple, RID &rid) override;

	const Schema &GetOutputSchema() const override {
		return plan_->OutputSchema();
	}

private:
	std::unique_ptr<SeqScanPlanNode> plan_;
};
} // namespace db
