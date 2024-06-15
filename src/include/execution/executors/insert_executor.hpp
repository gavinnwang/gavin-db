#pragma once

#include "execution/executors/abstract_executor.hpp"
#include "execution/plans/insert_plan.hpp"
namespace db {

class InsertExecutor : public AbstractExecutor {
public:
	InsertExecutor(const InsertPlanNode &plan, std::unique_ptr<AbstractExecutor> &&child_executor);
	// void Init() override;
	auto Next(Tuple &tuple, RID &rid) -> bool override;
	const Schema &GetOutputSchema() const override {
		return plan_.OutputSchema();
	};

private:
	std::unique_ptr<AbstractExecutor> child_executor_;
	const InsertPlanNode &plan_;
};
} // namespace db
