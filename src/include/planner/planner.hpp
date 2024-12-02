#pragma once

#include "binder/statement/bound_statement.hpp"
#include "binder/statement/insert_statement.hpp"
#include "binder/statement/select_statement.hpp"
#include "binder/table_ref/bound_expression_list.hpp"
#include "catalog/catalog_manager.hpp"
#include "execution/plans/abstract_plan.hpp"

#include <memory>
namespace db {
class Planner {
public:
	explicit Planner(std::unique_ptr<CatalogManager> &catalog_manager) : catalog_manager_(catalog_manager) {};

	void PlanQuery(const BoundStatement &statement);

	AbstractPlanNodeRef PlanSelect(const SelectStatement &statement);
	AbstractPlanNodeRef PlanInsert(const InsertStatement &statement);
	AbstractPlanNodeRef PlanTableRef(const BoundTableRef &table_ref);
	AbstractPlanNodeRef PlanExpressionListRef(const BoundExpressionListRef &table_ref);

private:
	/** the root plan node of the plan tree */
	AbstractPlanNodeRef plan_;

	[[maybe_unused]] std::unique_ptr<CatalogManager> &catalog_manager_;
};
} // namespace db
