#pragma once

#include "execution/executor_context.hpp"
#include "execution/executors/abstract_executor.hpp"
#include "execution/plans/abstract_plan.hpp"

#include <memory>

namespace db {
/** ExecutorFactory creates executors for arbitrary plan nodes. */
class ExecutorFactory {
public:
	// Creates a new executor given the executor context and plan node.
	static std::unique_ptr<AbstractExecutor> CreateExecutor(const ExecutorContext &exec_ctx, AbstractPlanNodeRef &plan);
};
} // namespace db
