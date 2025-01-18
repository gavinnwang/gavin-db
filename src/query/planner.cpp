#include "query/planner.hpp"

#include "../../cmake-build-debug/_deps/fmt-src/include/fmt/core.h"
#include "../../cmake-build-debug/_deps/magic_enum-src/include/magic_enum/magic_enum.hpp"
#include "../include/query/binder/expressions/bound_constant.hpp"
#include "../include/query/binder/table_ref/bound_expression_list.hpp"
#include "../include/common/exception.hpp"
#include "../include/common/macros.hpp"
#include "query/expressions/abstract_expression.hpp"
#include "query/expressions/constant_value_expression.hpp"
#include "query/plans/insert_plan.hpp"
#include "query/plans/seq_scan_plan.hpp"
#include "query/plans/values_plan.hpp"

#include <vector>

namespace db {

void Planner::PlanQuery(const BoundStatement &statement) {
	AbstractPlanNodeRef plan;
	switch (statement.type_) {
	case StatementType::SELECT_STATEMENT: {
		plan = PlanSelect(dynamic_cast<const SelectStatement &>(statement));
		break;
	}
	case StatementType::INSERT_STATEMENT: {
		plan = PlanInsert(dynamic_cast<const InsertStatement &>(statement));
		break;
	}
	default:
		throw NotImplementedException("Statement is not supported");
	}
	LOG_TRACE("{}", plan->ToString());
	plan_ = std::move(plan);
}

AbstractExpressionRef Planner::PlanConstant(const BoundConstant &expr) {
	return std::make_unique<ConstantValueExpression>(expr.val_);
}

AbstractExpressionRef Planner::PlanExpression(const BoundExpression &expr,
                                              [[maybe_unused]] const std::vector<AbstractPlanNodeRef> &children) {
	switch (expr.type_) {
	case ExpressionType::CONSTANT: {
		const auto &constant_expr = dynamic_cast<const BoundConstant &>(expr);
		return PlanConstant(constant_expr);
	}
	case ExpressionType::STAR:
	case ExpressionType::COLUMN_REF:
	case ExpressionType::TYPE_CAST:
	case ExpressionType::FUNCTION:
	case ExpressionType::AGG_CALL:
	case ExpressionType::UNARY_OP:
	case ExpressionType::BINARY_OP:
	case ExpressionType::ALIAS:
	case ExpressionType::FUNC_CALL:
	case ExpressionType::WINDOW:
	case ExpressionType::INVALID:
	default:
		throw NotImplementedException(
		    fmt::format("Not supported expression type {}", magic_enum::enum_name(expr.type_)));
	}
}

AbstractPlanNodeRef Planner::PlanExpressionListRef(const BoundExpressionListRef &table_ref) {
	std::vector<std::vector<AbstractExpressionRef>> all_exprs;
	for (const auto &row : table_ref.values_) {
		std::vector<AbstractExpressionRef> row_exprs;
		for (const auto &col : row) {
			auto expr = PlanExpression(*col, {});
			row_exprs.push_back(std::move(expr));
		}
		all_exprs.emplace_back(std::move(row_exprs));
	}

	const auto &first_row = all_exprs[0];
	std::vector<Column> cols;
	cols.reserve(first_row.size());
	size_t idx = 0;
	for (const auto &col : first_row) {
		auto col_name = fmt::format("Temp Col{}", idx);
		if (col->GetReturnType() != TypeId::VARCHAR) {
			cols.emplace_back(col_name, col->GetReturnType());
		} else {
			cols.emplace_back(col_name, col->GetReturnType(), VARCHAR_DEFAULT_LENGTH);
		}
		idx += 1;
	}
	auto schema = std::make_unique<Schema>(cols);

	return std::make_unique<ValuesPlanNode>(std::move(schema), std::move(all_exprs));
}

AbstractPlanNodeRef Planner::PlanTableRef(const BoundTableRef &table_ref) {
	// use to string here to trigger the virtual method
	// something goes wrong with the overriding of fmt formmatter
	LOG_TRACE("Planning table ref {}", table_ref.ToString());
	switch (table_ref.type_) {
	case TableReferenceType::EXPRESSION_LIST: {
		const auto &expression_list = dynamic_cast<const BoundExpressionListRef &>(table_ref);
		return PlanExpressionListRef(expression_list);
	}
	case TableReferenceType::BASE_TABLE: {
		const auto &base_table_ref = dynamic_cast<const BoundBaseTableRef &>(table_ref);
		auto &table = catalog_manager_.GetTableByName(base_table_ref.table_);
		std::vector<Column> cols;
		for (const auto &col : table.schema_.GetColumns()) {
			cols.emplace_back(col);
		}
		return std::make_unique<SeqScanPlanNode>(std::make_unique<Schema>(cols), table.table_oid_, table.name_,
		                                         nullptr);
	}
	case TableReferenceType::INVALID:
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
	LOG_TRACE("Planning select statement");
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

	// projection
	return plan;
}

AbstractPlanNodeRef Planner::PlanInsert(const InsertStatement &statement) {
	LOG_TRACE("Planning insert statement");
	auto select = PlanSelect(*statement.select_);

	const auto &table_schema = statement.table_->schema_;
	const auto &child_schema = select->OutputSchema();
	if (!Schema::TypeMatch(table_schema, child_schema)) {
		throw Exception(
		    fmt::format("Table schema mismatch for insert statement: {} vs {}", table_schema, child_schema));
	}

	auto insert_schema = std::make_unique<Schema>(std::vector {Column("inserted_rows", TypeId::INTEGER)});

	return std::make_unique<InsertPlanNode>(std::move(insert_schema), std::move(select), statement.table_->oid_);
}
} // namespace db
