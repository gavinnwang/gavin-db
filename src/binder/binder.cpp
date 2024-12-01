#include "binder/binder.hpp"

#include "binder/expressions/bound_columnn_ref.hpp"
#include "binder/expressions/bound_constant.hpp"
#include "binder/table_ref/bound_expression_list.hpp"
#include "binder/table_ref/bound_table_ref.hpp"
#include "common/exception.hpp"
#include "common/logger.hpp"
#include "sql/ColumnType.h"
#include "sql/SQLStatement.h"
#include "util/sqlhelper.h"

#include <memory>
#include <utility>

namespace db {
std::unique_ptr<BoundStatement> Binder::Bind(const hsql::SQLStatement *stmt) {
	switch (stmt->type()) {
	case hsql::kStmtCreate: {
		const auto *create_stmt = static_cast<const hsql::CreateStatement *>(stmt);
		return BindCreate(create_stmt);
	}
	case hsql::kStmtInsert: {
		const auto *insert_stmt = static_cast<const hsql::InsertStatement *>(stmt);
		return BindInsert(insert_stmt);
	}
	default:
		throw NotImplementedException("This statement is not supported");
	}
}

std::unique_ptr<CreateStatement> Binder::BindCreate(const hsql::CreateStatement *stmt) {
	auto table = std::string(stmt->tableName);
	auto columns = std::vector<Column> {};
	std::vector<std::string> primary_key;
	if (stmt->type == hsql::CreateType::kCreateTable) {
		for (const auto col_def : *stmt->columns) {
			auto col = BindColumnDefinition(col_def);
			if (col_def->column_constraints->contains(hsql::ConstraintType::PrimaryKey)) {
				primary_key.push_back(col.GetName());
			}
			if (primary_key.size() > 1) {
				throw NotImplementedException("Cannot have two primary keys");
			}
			columns.emplace_back(col);
		}
	} else {
		throw NotImplementedException("Unsupported create statement");
	}
	return std::make_unique<CreateStatement>(std::move(table), std::move(columns), std::move(primary_key));
}

std::unique_ptr<SelectStatement> Binder::BindSelect(const hsql::SelectStatement *stmt) {
	LOG_TRACE("Binding select statement");
	ASSERT(stmt, "Select statement cannot be nullptr");
	hsql::printSelectStatementInfo(stmt, 1);

	// Bind VALUES clause.
	if (stmt->selectList != nullptr) {
		auto value_list = BindValuesList(std::as_const(*stmt->selectList));
		std::vector<std::unique_ptr<BoundExpression>> exprs;
		size_t expr_length = value_list->values_[0].size();
		for (size_t i = 0; i < expr_length; i++) {
			exprs.emplace_back(std::make_unique<BoundColumnRef>(std::vector {fmt::format("Temp Col {}", i)}));
		}
		return std::make_unique<SelectStatement>(std::move(value_list), std::move(exprs));
	}
	throw NotImplementedException("Have not implemented select clause like this");
}

std::unique_ptr<InsertStatement> Binder::BindInsert(const hsql::InsertStatement *stmt) {
	LOG_TRACE("Binding insert statement");
	hsql::printInsertStatementInfo(stmt, 1);
	auto table = BindBaseTableRef(stmt->tableName);

	if (table->GetBoundTableName().starts_with("__")) {
		throw Exception(fmt::format("invalid table for insert: {}", table->table_));
	}

	if (stmt->values) {
		auto value_list = BindValuesList(*stmt->values);
		LOG_TRACE("{}", value_list->ToString());
		std::vector<std::unique_ptr<BoundExpression>> exprs;
		size_t expr_length = value_list->values_[0].size();
		for (size_t i = 0; i < expr_length; i++) {
			exprs.emplace_back(std::make_unique<BoundColumnRef>(std::vector {fmt::format("Temp Col {}", i)}));
		}
		auto select = std::make_unique<SelectStatement>(std::move(value_list), std::move(exprs));
		return std::make_unique<InsertStatement>(std::move(table), std::move(select));
	}

	// ASSERT(stmt->select, "select is nullptr");
	// auto select = BindSelect(stmt->select);
	// return std::make_unique<InsertStatement>(std::move(table), std::move(select));
	throw NotImplementedException("Have not implemented insert clause like this");
}

std::unique_ptr<BoundExpression> Binder::BindExpression(const hsql::Expr *expr) {
	switch (expr->type) {
	case hsql::kExprLiteralInt: {
		LOG_TRACE("int expr {}", expr->ival);
		auto IntValue = Value {TypeId::INTEGER, static_cast<int32_t>(expr->ival)};
		return std::make_unique<BoundConstant>(std::move(IntValue));
	}
	case hsql::kExprLiteralString: {
		std::string string {expr->getName()};
		LOG_TRACE("string expr {}", string);
		auto VarcharValue = Value {TypeId::VARCHAR, std::move(string)};
		return std::make_unique<BoundConstant>(std::move(VarcharValue));
	}
	case hsql::kExprLiteralFloat:
	case hsql::kExprLiteralNull:
	case hsql::kExprLiteralDate:
	case hsql::kExprLiteralInterval:
	case hsql::kExprStar:
	case hsql::kExprParameter:
	case hsql::kExprColumnRef:
	case hsql::kExprFunctionRef:
	case hsql::kExprOperator:
	case hsql::kExprSelect:
	case hsql::kExprHint:
	case hsql::kExprArray:
	case hsql::kExprArrayIndex:
	case hsql::kExprExtract:
	case hsql::kExprCast:
	default:
		throw NotImplementedException("This expr type is not supported");
	}
	std::unreachable();
}
std::vector<std::unique_ptr<BoundExpression>> Binder::BindExpressionList(const std::vector<hsql::Expr *> &list) {
	std::vector<std::unique_ptr<BoundExpression>> expr_list;
	expr_list.reserve(list.size());
	for (const auto *expr : list) {
		expr_list.emplace_back(BindExpression(expr));
	}
	return expr_list;
}

std::unique_ptr<BoundExpressionListRef> Binder::BindValuesList(const std::vector<hsql::Expr *> &list) {
	std::vector<std::vector<std::unique_ptr<BoundExpression>>> value_list {};
	// Currently value list only support one value
	value_list.emplace_back(BindExpressionList(list));
	return std::make_unique<BoundExpressionListRef>(std::move(value_list));
}

std::unique_ptr<BoundBaseTableRef> Binder::BindBaseTableRef(const std::string &table_name) {
	const auto &table_info = catalog_manager_->GetTableByName(table_name);
	if (table_info == nullptr) {
		throw Exception(fmt::format("invalid table {}", table_name));
	}
	return std::make_unique<BoundBaseTableRef>(std::move(table_name), table_info->table_oid_, table_info->schema_);
}

Column Binder::BindColumnDefinition(const hsql::ColumnDefinition *col_def) const {
	auto col_type = col_def->type;
	std::string col_name = std::string(col_def->name);
	const auto col = (col_type.data_type == hsql::DataType::VARCHAR)
	                     ? Column {std::move(col_name), Type::HsqlColumnTypeToTypeId(col_type),
	                               static_cast<uint32_t>(col_def->type.length)}
	                     : Column {std::move(col_name), Type::HsqlColumnTypeToTypeId(col_type)};

	LOG_TRACE("Binding Column Def: {}", col.ToString());
	return col;
}

} // namespace db
