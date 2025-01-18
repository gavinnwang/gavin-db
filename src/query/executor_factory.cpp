#include "query/executor_factory.hpp"

#include "common/exception.hpp"
#include "query/executors/insert_executor.hpp"
#include "query/executors/seq_scan_executor.hpp"
#include "query/executors/value_executor.hpp"
#include "query/plans/insert_plan.hpp"
#include "query/plans/seq_scan_plan.hpp"
#include "query/plans/values_plan.hpp"
#include "fmt/core.h"
#include "magic_enum/magic_enum.hpp"

#include <memory>
namespace db {

[[nodiscard]] std::unique_ptr<AbstractExecutor> CreateInsertExecutor(const ExecutorContext &exec_ctx,
                                                                     std::unique_ptr<InsertPlanNode> plan) {
	LOG_TRACE("Creating insert executor");
	auto child_executor = ExecutorFactory::CreateExecutor(exec_ctx, std::move(plan->GetChildPlan()));
	return std::make_unique<InsertExecutor>(exec_ctx, std::move(plan), std::move(child_executor));
}

[[nodiscard]] std::unique_ptr<AbstractExecutor> CreateValuesExecutor(const ExecutorContext &exec_ctx,
                                                                     std::unique_ptr<ValuesPlanNode> plan) {
	LOG_TRACE("Creating values executor");
	return std::make_unique<ValuesExecutor>(exec_ctx, std::move(plan));
}

[[nodiscard]] std::unique_ptr<AbstractExecutor> CreateSeqScanExecutor(const ExecutorContext &exec_ctx,
                                                                      std::unique_ptr<SeqScanPlanNode> plan) {
	LOG_TRACE("Creating seq scan executor");
	return std::make_unique<SeqScanExecutor>(exec_ctx, std::move(plan));
}

[[nodiscard]] std::unique_ptr<AbstractExecutor> ExecutorFactory::CreateExecutor(const ExecutorContext &exec_ctx,
                                                                                AbstractPlanNodeRef plan) {
	switch (plan->GetType()) {
	case PlanType::Insert:
		return CreateInsertExecutor(exec_ctx,
		                            std::unique_ptr<InsertPlanNode>(static_cast<InsertPlanNode *>(plan.release())));
	case PlanType::Values:
		return CreateValuesExecutor(exec_ctx,
		                            std::unique_ptr<ValuesPlanNode>(static_cast<ValuesPlanNode *>(plan.release())));
	case PlanType::SeqScan:
		return CreateSeqScanExecutor(exec_ctx,
		                             std::unique_ptr<SeqScanPlanNode>(static_cast<SeqScanPlanNode *>(plan.release())));
	default:
		throw NotImplementedException(fmt::format("Plan not supported {}", magic_enum::enum_name(plan->GetType())));
	}
}
} // namespace db
