#pragma once

#include "execution/expressions/abstract_expression.hpp"
#include "execution/plans/abstract_plan.hpp"
namespace db {

class ProjectionPlanNode : public AbstractPlanNode {
public:
	ProjectionPlanNode(SchemaRef output, std::vector<AbstractExpressionRef> expressions, AbstractPlanNodeRef child)
	    : AbstractPlanNode(std::move(output), std::move(child)), expressions_(std::move(expressions)) {
	}
	PlanType GetType() const override {
		return PlanType::Projection;
	}
	const AbstractPlanNodeRef &GetChildPlan() const {
		assert(GetChildren().size() == 1);
		return children_.at(0);
	}

	const std::vector<AbstractExpressionRef> &GetExpressions() const {
		return expressions_;
	}

	static Schema InferProjectionSchema(const std::vector<AbstractExpressionRef> &expressions);

	static Schema RenameSchema(const Schema &schema, const std::vector<std::string> &col_names);

private:
	std::vector<AbstractExpressionRef> expressions_;
};

} // namespace db
