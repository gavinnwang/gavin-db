#pragma once

#include "common/typedef.hpp"
#include "execution/plans/abstract_plan.hpp"
#include "fmt/ranges.h"
namespace db {

// only has one child
class InsertPlanNode : public AbstractPlanNode {
public:
	InsertPlanNode(SchemaRef output, AbstractPlanNodeRef child, table_oid_t table_oid)
	    : AbstractPlanNode(std::move(output), std::move(child)), table_oid_(table_oid) {
	}
	[[nodiscard]] PlanType GetType() const override {
		return PlanType::Insert;
	}
	[[nodiscard]] table_oid_t GetTableOid() const {
		return table_oid_;
	}
	AbstractPlanNodeRef &GetChildPlan() {
		assert(GetChildren().size() == 1);
		return children_.at(0);
	}
	[[nodiscard]] std::string ToString() const override {
		std::string children;
		for (const auto &child : children_) {
			children += child->ToString();
		}
		return fmt::format("InsertPlanNode {{ table_oid={}, children={} }}", table_oid_, children);
	}

private:
	table_oid_t table_oid_;

protected:
};

} // namespace db
