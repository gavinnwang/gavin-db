#pragma once

#include "common/typedef.hpp"
#include "execution/plans/abstract_plan.hpp"
#include "fmt/core.h"
namespace db {

class InsertPlanNode : public AbstractPlanNode {
public:
	InsertPlanNode(SchemaRef output, AbstractPlanNodeRef child, table_oid_t table_oid)
	    : AbstractPlanNode(std::move(output), {std::move(child)}), table_oid_(table_oid) {
	}
	auto ToString() const -> std::string override {
		return fmt::format("Insert {{ table_oid={} }}", table_oid_);
	}

private:
	table_oid_t table_oid_;

protected:
};

} // namespace db
