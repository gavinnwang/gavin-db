#pragma once

#include "execution/executor_context.hpp"
#include "execution/executors/abstract_executor.hpp"
#include "execution/plans/insert_plan.hpp"

namespace db {

class InsertExecutor : public AbstractExecutor {
public:
	InsertExecutor(const std::unique_ptr<ExecutorContext> &exec_context, std::unique_ptr<InsertPlanNode> plan,
	               std::unique_ptr<AbstractExecutor> &&child_executor)
	    : AbstractExecutor(exec_context), child_executor_(std::move(child_executor)), plan_(std::move(plan)) {
	}

	auto Next(Tuple &tuple, RID &rid) -> bool override;

	const Schema &GetOutputSchema() const override {
		return plan_->OutputSchema();
	};

private:
	std::unique_ptr<AbstractExecutor> child_executor_;
	const std::unique_ptr<InsertPlanNode> plan_;
	bool executed_ = false;
};
} // namespace db
