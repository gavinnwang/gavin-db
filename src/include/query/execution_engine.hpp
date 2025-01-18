#pragma once

#include "common/rid.hpp"
#include "concurrency/transaction.hpp"
#include "query/executor_factory.hpp"
#include "query/executors/abstract_executor.hpp"
#include "query/plans/abstract_plan.hpp"
#include "storage/table/tuple.hpp"
namespace db {

class ExecutionEngine {
public:
	ExecutionEngine() = default;
	ExecutionEngine(const ExecutionEngine &) = delete;
	ExecutionEngine &operator=(const ExecutionEngine &) = delete;
	ExecutionEngine(ExecutionEngine &&) = delete;
	ExecutionEngine &operator=(ExecutionEngine &&) = delete;

	static void Execute(AbstractPlanNodeRef plan, std::vector<Tuple> &result_set, [[maybe_unused]] Transaction &txn,
	                    const ExecutorContext &exec_ctx) {
		auto executor = ExecutorFactory::CreateExecutor(exec_ctx, std::move(plan));
		PollExecutor(executor, result_set);
	}

private:
	static void PollExecutor(std::unique_ptr<AbstractExecutor> &executor, std::vector<Tuple> &result_set) {
		RID rid {};
		Tuple tuple {};
		while (executor->Next(tuple, rid)) {
			result_set.push_back(tuple);
		}
	}
};

} // namespace db
