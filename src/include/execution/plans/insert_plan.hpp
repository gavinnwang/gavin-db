#pragma once

#include "common/typedef.hpp"
#include "execution/plans/abstract_plan.hpp"
#include "fmt/core.h"
namespace db {

// only has one child
class InsertPlanNode : public AbstractPlanNode {
public:
	InsertPlanNode(SchemaRef output, AbstractPlanNodeRef child, table_oid_t table_oid)
	    : AbstractPlanNode(std::move(output), std::move(child)), table_oid_(table_oid) {
	}
	PlanType GetType() const override {
		return PlanType::Insert;
	}
	table_oid_t GetTableOid() const {
		return table_oid_;
	}
	const AbstractPlanNodeRef &GetChildPlan() const {
		assert(GetChildren().size() == 1);
		return GetChildAt(0);
	}
	std::string ToString() const override {
		return fmt::format("Insert {{ table_oid={} }}", table_oid_);
	}

private:
	table_oid_t table_oid_;

protected:
};

} // namespace db
