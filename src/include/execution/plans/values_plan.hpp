#pragma once

#include "execution/expressions/abstract_expression.hpp"
#include "execution/plans/abstract_plan.hpp"

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
namespace db {

class ValuesPlanNode : public AbstractPlanNode {
public:
	explicit ValuesPlanNode(SchemaRef output, std::vector<std::vector<AbstractExpressionRef>> values)
	    : AbstractPlanNode {std::move(output)}, values_ {std::move(values)} {
	}

	[[nodiscard]] PlanType GetType() const override {
		return PlanType::Values;
	}

	[[nodiscard]] const AbstractPlanNodeRef &GetChildPlan() const {
		assert(GetChildren().size() == 1);
		return children_.at(0);
	}

	[[nodiscard]] const std::vector<std::vector<AbstractExpressionRef>> & GetValues() const   {
		return values_;
	}

	[[nodiscard]] std::string ToString() const override {
		std::string values = "{ ";
		for (const auto &value_list : values_) {
			values += "[";
			for (const auto &value : value_list) {
				values += value->ToString() + ", ";
			}
			if (!value_list.empty()) {
				values.pop_back();
				values.pop_back();
			}
			values += "], ";
		}
		if (!values_.empty()) {
			values.pop_back();
			values.pop_back();
		}
		values += " }";

		return fmt::format("ValuesPlanNode {{ values={} }}", values);
	}

private:
	std::vector<std::vector<AbstractExpressionRef>> values_;

protected:
};
} // namespace db
