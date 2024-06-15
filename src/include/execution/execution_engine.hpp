#pragma once

#include "execution/plans/abstract_plan.hpp"
namespace db {

class ExecutionEngine {
public:
	ExecutionEngine(AbstractPlanNodeRef plan) : plan_(std::move(plan)) {
	}
	DISALLOW_COPY_AND_MOVE(ExecutionEngine);

private:
	AbstractPlanNodeRef plan_;
};

} // namespace db
