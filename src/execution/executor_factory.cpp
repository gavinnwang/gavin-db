#include "execution/executor_factory.hpp"

#include "common/exception.hpp"
#include "execution/executors/insert_executor.hpp"
#include "execution/executors/value_executor.hpp"
#include "execution/plans/insert_plan.hpp"
#include "execution/plans/values_plan.hpp"
#include "fmt/core.h"
#include "magic_enum/magic_enum.hpp"

#include <memory>
namespace db {

std::unique_ptr<AbstractExecutor> ExecutorFactory::CreateExecutor(const std::unique_ptr<ExecutorContext> &exec_ctx,
                                                                  AbstractPlanNodeRef &plan) {
	switch (plan->GetType()) {
	case PlanType::Insert: {
		LOG_TRACE("Creating insert executor");
		auto *raw_insert_plan = dynamic_cast<InsertPlanNode *>(plan.release());
		std::unique_ptr<InsertPlanNode> insert_plan(raw_insert_plan);
		auto child_executor = ExecutorFactory::CreateExecutor(exec_ctx, insert_plan->GetChildPlan());
		return std::make_unique<InsertExecutor>(exec_ctx, std::move(insert_plan), std::move(child_executor));
	}
	case PlanType::Values: {
		LOG_TRACE("Creating values executor");
		auto *raw_values_plan = dynamic_cast<ValuesPlanNode *>(plan.release());
		std::unique_ptr<ValuesPlanNode> values_plan(raw_values_plan);
		return std::make_unique<ValuesExecutor>(exec_ctx, std::move(values_plan));
	}
	default:
		throw NotImplementedException(fmt::format("Plan not supported {}", magic_enum::enum_name(plan->GetType())));
	}
}
} // namespace db
