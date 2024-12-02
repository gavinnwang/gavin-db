#pragma once

#include "execution/executor_context.hpp"
#include "execution/executors/abstract_executor.hpp"
#include "execution/plans/values_plan.hpp"
#include "storage/table/tuple.hpp"

#include <memory>

namespace db {

class ValuesExecutor : public AbstractExecutor {
public:
	ValuesExecutor(const ExecutorContext &exec_ctx, std::unique_ptr<ValuesPlanNode> plan)
	    : AbstractExecutor(exec_ctx), plan_(std::move(plan)) {
	}

	bool Next(Tuple &tuple, RID &rid) override;

	const Schema &GetOutputSchema() const override {
		return plan_->OutputSchema();
	};

private:
	std::unique_ptr<ValuesPlanNode> plan_;
	idx_t cursor_ {0};
};
} // namespace db
