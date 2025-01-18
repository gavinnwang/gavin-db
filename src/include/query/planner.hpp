#pragma once

#include "query/binder/expressions/bound_constant.hpp"
#include "query/binder/statement/bound_statement.hpp"
#include "query/binder/statement/insert_statement.hpp"
#include "query/binder/statement/select_statement.hpp"
#include "query/binder/table_ref/bound_expression_list.hpp"
#include "meta/catalog_manager.hpp"
#include "query/expressions/abstract_expression.hpp"
#include "query/plans/abstract_plan.hpp"

#include <memory>
namespace db {
class Planner {
public:
	explicit Planner(CatalogManager &catalog_manager) : catalog_manager_(catalog_manager) {};

	void PlanQuery(const BoundStatement &statement);

	AbstractPlanNodeRef PlanSelect(const SelectStatement &statement);
	AbstractPlanNodeRef PlanInsert(const InsertStatement &statement);
	AbstractPlanNodeRef PlanTableRef(const BoundTableRef &table_ref);
	AbstractExpressionRef PlanConstant(const BoundConstant &expr);
	AbstractExpressionRef PlanExpression(const BoundExpression &expr, const std::vector<AbstractPlanNodeRef> &children);
	AbstractPlanNodeRef PlanExpressionListRef(const BoundExpressionListRef &table_ref);

	/** the root plan node of the plan tree */
	AbstractPlanNodeRef plan_;

private:
	CatalogManager &catalog_manager_;
};
} // namespace db
