#pragma once

#include "common/rid.hpp"
#include "execution/executors/abstract_executor.hpp"
#include "execution/plans/abstract_plan.hpp"
#include "storage/table/tuple.hpp"
namespace db {

class ExecutionEngine {
public:
	ExecutionEngine(AbstractPlanNodeRef plan) : plan_(std::move(plan)) {
	}
	DISALLOW_COPY_AND_MOVE(ExecutionEngine);

private:
	static void PollExecutor(std::unique_ptr<AbstractExecutor> &executor, std::vector<Tuple> &result_set) {
		RID rid {};
		Tuple tuple {};
		while (executor->Next(tuple, rid)) {
			result_set.push_back(tuple);
		}
	}
	AbstractPlanNodeRef plan_;
};

} // namespace db
