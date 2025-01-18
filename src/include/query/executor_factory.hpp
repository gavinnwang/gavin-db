#pragma once

#include "query/executor_context.hpp"
#include "query/executors/abstract_executor.hpp"
#include "query/plans/abstract_plan.hpp"

#include <memory>

namespace db {
/** ExecutorFactory creates executors for arbitrary plan nodes. */
class ExecutorFactory {
public:
	// Creates a new executor given the executor context and plan node.
	static std::unique_ptr<AbstractExecutor> CreateExecutor(const ExecutorContext &exec_ctx, AbstractPlanNodeRef plan);
};
} // namespace db
