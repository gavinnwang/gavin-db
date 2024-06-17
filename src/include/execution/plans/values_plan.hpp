#pragma once

#include "execution/expressions/abstract_expression.hpp"
#include "execution/plans/abstract_plan.hpp"
#include "fmt/core.h"
namespace db {

class ValuesPlanNode : public AbstractPlanNode {
public:
	explicit ValuesPlanNode(SchemaRef output, std::vector<std::vector<AbstractExpressionRef>> values)
	    : AbstractPlanNode {std::move(output)}, values_ {std::move(values)} {
	}

	PlanType GetType() const override {
		return PlanType::Values;
	}
	const AbstractPlanNodeRef &GetChildPlan() const {
		assert(GetChildren().size() == 1);
		return GetChildAt(0);
	}
	std::string ToString() const override {
		return fmt::format("Values {{ }}");
	}

private:
	std::vector<std::vector<AbstractExpressionRef>> values_;

protected:
};
} // namespace db
