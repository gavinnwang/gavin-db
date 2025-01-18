#include "query/binder/binder.hpp"

#include "common/arithmetic_type.hpp"
#include "common/exception.hpp"
#include "common/logger.hpp"
#include "magic_enum/magic_enum.hpp"
#include "query/binder/expressions/bound_binary_op.hpp"
#include "query/binder/expressions/bound_columnn_ref.hpp"
#include "query/binder/expressions/bound_constant.hpp"
#include "query/binder/expressions/bound_star.hpp"
#include "query/binder/table_ref/bound_expression_list.hpp"
#include "query/binder/table_ref/bound_table_ref.hpp"
#include "sql/ColumnType.h"
#include "sql/Expr.h"
#include "sql/SQLStatement.h"
#include "util/sqlhelper.h"

#include <memory>
#include <utility>

namespace db {
std::unique_ptr<BoundStatement> Binder::Bind(const hsql::SQLStatement *stmt) {
	switch (stmt->type()) {
	case hsql::kStmtCreate: {
		const auto *create_stmt = dynamic_cast<const hsql::CreateStatement *>(stmt);
		return BindCreate(create_stmt);
	}
	case hsql::kStmtInsert: {
		const auto *insert_stmt = dynamic_cast<const hsql::InsertStatement *>(stmt);
		return BindInsert(insert_stmt);
	}
	case ::hsql::kStmtSelect: {
		const auto *select_stmt = dynamic_cast<const hsql::SelectStatement *>(stmt);
		return BindSelect(select_stmt);
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
		for (auto *const col_def : *stmt->columns) {
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
std::unique_ptr<BoundTableRef> Binder::BindFrom(const hsql::TableRef *table_ref) {
	return BindBaseTableRef(table_ref->getName());
}

std::unique_ptr<SelectStatement> Binder::BindSelect(const hsql::SelectStatement *stmt) {
	LOG_TRACE("Binding select statement");
	assert(stmt && "Select statement cannot be nullptr");
	hsql::printSelectStatementInfo(stmt, 1);

	// Bind SELECT VALUES clause.
	if (stmt->fromTable == nullptr && stmt->selectList != nullptr) {
		LOG_TRACE("Binding SELECT VALUES clause");
		auto table_value_list = BindValuesList(*stmt->selectList);
		std::vector<std::unique_ptr<BoundExpression>> select_list;
		size_t expr_length = table_value_list->values_[0].size();
		for (size_t i = 0; i < expr_length; i++) {
			select_list.emplace_back(std::make_unique<BoundColumnRef>(std::vector {fmt::format("Temp Col {}", i)}));
		}
		return std::make_unique<SelectStatement>(std::move(table_value_list), std::move(select_list));
	}

	// Bind select __ from __ caluse.
	if (stmt->fromTable != nullptr && stmt->selectList != nullptr) {
		LOG_TRACE("Binding SELECT __ FROM __ clause");
		auto from_table = BindFrom(stmt->fromTable);
		scope_ = &from_table;
		auto select_list = BindExpressionList(*stmt->selectList);
		LOG_TRACE("from table: {}, select list {}", from_table, select_list);
		return std::make_unique<SelectStatement>(std::move(from_table), std::move(select_list));
	}
	throw NotImplementedException("Have not implemented select clause like this");
}

std::unique_ptr<InsertStatement> Binder::BindInsert(const hsql::InsertStatement *stmt) {
	assert(stmt && "Insert statement cannot be nullptr");
	LOG_TRACE("Binding insert statement");
	hsql::printInsertStatementInfo(stmt, 1);
	auto table = BindBaseTableRef(stmt->tableName);

	if (table->GetBoundTableName().starts_with("__")) {
		throw Exception(fmt::format("invalid table for insert: {}", table->table_));
	}

	if (stmt->values != nullptr) {
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
	if (stmt->select != nullptr) {
		auto select = BindSelect(stmt->select);
		return std::make_unique<InsertStatement>(std::move(table), std::move(select));
	}

	// ASSERT(stmt->select, "select is nullptr");
	throw NotImplementedException("Have not implemented insert clause like this");
}

std::unique_ptr<BoundExpression> Binder::BindExpression(const hsql::Expr *expr) {
	switch (expr->type) {
	case hsql::kExprLiteralInt: {
		LOG_TRACE("int expr {}", expr->ival);
		auto int_value = Value {TypeId::INTEGER, static_cast<int32_t>(expr->ival)};
		return std::make_unique<BoundConstant>(std::move(int_value));
	}
	case hsql::kExprLiteralString: {
		std::string string {expr->getName()};
		LOG_TRACE("string expr {}", string);
		auto varchar_value = Value {TypeId::VARCHAR, std::move(string)};
		return std::make_unique<BoundConstant>(std::move(varchar_value));
	}
	case hsql::kExprOperator: {
		ArithmeticType op_type;
		switch (expr->opType) {
		case hsql::OperatorType::kOpPlus: {
			op_type = ArithmeticType::Plus;
			break;
		}
		case hsql::OperatorType::kOpMinus: {
			op_type = ArithmeticType::Minus;
			break;
		}
		case hsql::OperatorType::kOpAsterisk: {
			op_type = ArithmeticType::Multiply;
			break;
		}
		default:
			throw NotImplementedException(
			    fmt::format("Operator type is not supported {}", magic_enum::enum_name(expr->opType)));
		}
		auto larg = BindExpression(expr->expr);
		auto rarg = BindExpression(expr->expr2);
		return std::make_unique<BoundBinaryOp>(op_type, std::move(larg), std::move(rarg));
	}
	case hsql::kExprStar: {
		return std::make_unique<BoundStar>();
	}
	case hsql::kExprColumnRef:
	case hsql::kExprSelect:
	case hsql::kExprLiteralFloat:
	case hsql::kExprLiteralNull:
	case hsql::kExprLiteralDate:
	case hsql::kExprLiteralInterval:
	case hsql::kExprParameter:
	case hsql::kExprFunctionRef:
	case hsql::kExprHint:
	case hsql::kExprArray:
	case hsql::kExprArrayIndex:
	case hsql::kExprExtract:
	case hsql::kExprCast:
	default:
		throw NotImplementedException(
		    fmt::format("This expr type is not supported {}", magic_enum::enum_name(expr->type)));
	}
	std::unreachable();
}
std::vector<std::unique_ptr<BoundExpression>> Binder::BindExpressionList(const std::vector<hsql::Expr *> &list) {
	std::vector<std::unique_ptr<BoundExpression>> expr_list;
	expr_list.reserve(list.size());
	for (const auto *expr : list) {
		auto bound_expr = BindExpression(expr);
		if (bound_expr->type_ == ExpressionType::STAR) {
			if (list.size() != 1) {
				throw Exception("select * cannot have other expressions in list");
			}
			const auto *base_table_ref = dynamic_cast<BoundBaseTableRef *>(scope_->get());
			auto bound_table_name = base_table_ref->GetBoundTableName();
			const auto &schema = base_table_ref->schema_;
			auto columns = std::vector<std::unique_ptr<BoundExpression>> {};
			for (const auto &column : schema.GetColumns()) {
				columns.push_back(std::make_unique<BoundColumnRef>(std::vector {bound_table_name, column.GetName()}));
			}
			return columns;
		}
		expr_list.emplace_back(std::move(bound_expr));
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
	auto &table_info = catalog_.GetTableByName(table_name);
	return std::make_unique<BoundBaseTableRef>(table_name, table_info.table_oid_, table_info.schema_);
}

Column Binder::BindColumnDefinition(const hsql::ColumnDefinition *col_def) const {
	auto col_type = col_def->type;
	std::string col_name = std::string(col_def->name);
	auto col = (col_type.data_type == hsql::DataType::VARCHAR)
	               ? Column {std::move(col_name), Type::HsqlColumnTypeToTypeId(col_type),
	                         static_cast<uint32_t>(col_def->type.length)}
	               : Column {std::move(col_name), Type::HsqlColumnTypeToTypeId(col_type)};

	LOG_TRACE("Binding Column Def: {}", col.ToString());
	return col;
}

} // namespace db
