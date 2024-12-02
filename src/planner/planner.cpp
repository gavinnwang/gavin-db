#include "planner/planner.hpp"

#include "binder/table_ref/bound_expression_list.hpp"
#include "common/exception.hpp"
#include "execution/expressions/abstract_expression.hpp"
#include "execution/plans/insert_plan.hpp"
#include "execution/plans/values_plan.hpp"

#include <vector>

namespace db {

void Planner::PlanQuery(const BoundStatement &statement) {
	switch (statement.type_) {
	case StatementType::SELECT_STATEMENT: {
		plan_ = PlanSelect(dynamic_cast<const SelectStatement &>(statement));
		return;
	}
	case StatementType::INSERT_STATEMENT: {
		plan_ = PlanInsert(dynamic_cast<const InsertStatement &>(statement));
		return;
	}
	default:
		throw NotImplementedException("Statement is not supported");
	}
}

// auto Planner::PlanTableRef(const BoundTableRef &table_ref) -> AbstractPlanNodeRef {
//   switch (table_ref.type_) {
//     case TableReferenceType::BASE_TABLE: {
//       const auto &base_table_ref = dynamic_cast<const BoundBaseTableRef &>(table_ref);
//       return PlanBaseTableRef(base_table_ref);
//     }
//     case TableReferenceType::CROSS_PRODUCT: {
//       const auto &cross_product = dynamic_cast<const BoundCrossProductRef &>(table_ref);
//       return PlanCrossProductRef(cross_product);
//     }
//     case TableReferenceType::JOIN: {
//       const auto &join = dynamic_cast<const BoundJoinRef &>(table_ref);
//       return PlanJoinRef(join);
//     }
//     case TableReferenceType::EXPRESSION_LIST: {
//       const auto &expression_list = dynamic_cast<const BoundExpressionListRef &>(table_ref);
//       return PlanExpressionListRef(expression_list);
//     }
//     case TableReferenceType::SUBQUERY: {
//       const auto &subquery = dynamic_cast<const BoundSubqueryRef &>(table_ref);
//       return PlanSubquery(subquery, subquery.alias_);
//     }
//     case TableReferenceType::CTE: {
//       const auto &cte = dynamic_cast<const BoundCTERef &>(table_ref);
//       return PlanCTERef(cte);
//     }
//     default:
//       break;
//   }
//   throw Exception(fmt::format("the table ref type {} is not supported in planner yet", table_ref.type_));
// }

auto Planner::PlanExpressionListRef([[maybe_unused]] const BoundExpressionListRef &table_ref) -> AbstractPlanNodeRef {
	return nullptr;
}

AbstractPlanNodeRef Planner::PlanTableRef(const BoundTableRef &table_ref) {
	switch (table_ref.type_) {
	case TableReferenceType::EXPRESSION_LIST: {
		const auto &expression_list = dynamic_cast<const BoundExpressionListRef &>(table_ref);
		return PlanExpressionListRef(expression_list);
	}
	case TableReferenceType::INVALID:
	case TableReferenceType::BASE_TABLE:
	case TableReferenceType::JOIN:
	case TableReferenceType::CROSS_PRODUCT:
	case TableReferenceType::SUBQUERY:
	case TableReferenceType::CTE:
	case TableReferenceType::EMPTY:
	default:
		break;
	}
	throw NotImplementedException("From clause not implemented");
}

AbstractPlanNodeRef Planner::PlanSelect(const SelectStatement &statement) {
	AbstractPlanNodeRef plan = nullptr;

	// plan from clause
	// from table or from value list
	switch (statement.table_->type_) {
	case TableReferenceType::EMPTY:
		std::vector<std::vector<AbstractExpressionRef>> {};
		plan = std::make_unique<ValuesPlanNode>(std::make_unique<Schema>(std::vector<Column> {}),
		                                        std::vector<std::vector<AbstractExpressionRef>> {});
		break;
	default:
		plan = PlanTableRef(*statement.table_);
		break;
	}
	return nullptr;
}

AbstractPlanNodeRef Planner::PlanInsert(const InsertStatement &statement) {
	auto select = PlanSelect(*statement.select_);

	const auto &table_schema = statement.table_->schema_.GetColumns();
	const auto &child_schema = select->OutputSchema().GetColumns();
	if (!std::equal(table_schema.cbegin(), table_schema.cend(), child_schema.cbegin(), child_schema.cend(),
	                [](auto &&col1, auto &&col2) { return col1.GetType() == col2.GetType(); })) {
		throw db::Exception("table schema mismatch");
	}

	auto insert_schema = std::make_unique<Schema>(std::vector {Column("inserted_rows", TypeId::INTEGER)});

	return std::make_unique<InsertPlanNode>(std::move(insert_schema), std::move(select), statement.table_->oid_);
}
} // namespace db
