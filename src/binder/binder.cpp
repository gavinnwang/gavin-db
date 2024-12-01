#include "binder/binder.hpp"

#include "binder/table_ref/bound_table_ref.hpp"
#include "common/exception.hpp"
#include "common/logger.hpp"
#include "sql/ColumnType.h"
#include "sql/SQLStatement.h"
#include "util/sqlhelper.h"

#include <memory>

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
	hsql::printSelectStatementInfo(stmt, 1);

	// if (stmt->selectList->empty()) {
	// 	return std::make_unique<BoundTableRef>(TableReferenceType::EMPTY);
	// }
	// // Bind VALUES clause.
	// if (pg_stmt->valuesLists != nullptr) {
	//   auto values_list_name = fmt::format("__values#{}", universal_id_++);
	//   auto value_list = BindValuesList(pg_stmt->valuesLists);
	//   value_list->identifier_ = values_list_name;
	//   std::vector<std::unique_ptr<BoundExpression>> exprs;
	//   size_t expr_length = value_list->values_[0].size();
	//   for (size_t i = 0; i < expr_length; i++) {
	//     exprs.emplace_back(std::make_unique<BoundColumnRef>(std::vector{values_list_name, fmt::format("{}", i)}));
	//   }
	//   return std::make_unique<SelectStatement>(
	//       std::move(value_list), std::move(exprs), std::make_unique<BoundExpression>(),
	//       std::vector<std::unique_ptr<BoundExpression>>{}, std::make_unique<BoundExpression>(),
	//       std::make_unique<BoundExpression>(), std::make_unique<BoundExpression>(),
	//       std::vector<std::unique_ptr<BoundOrderBy>>{}, std::vector<std::unique_ptr<BoundSubqueryRef>>{}, false);
	// }

	// Bind VALUES clause.
	if (stmt->selectList != nullptr) {
		auto value_list = BindValuesList(std::as_const(*stmt->selectList));
		std::vector<std::unique_ptr<BoundExpression>> exprs;
		size_t expr_length = value_list->values_[0].size();
		for (size_t i = 0; i < expr_length; i++) {
			exprs.emplace_back(std::make_unique<BoundColumnRef>(std::vector {values_list_name, fmt::format("{}", i)}));
		}
	}
}

std::unique_ptr<InsertStatement> Binder::BindInsert(const hsql::InsertStatement *stmt) {
	LOG_TRACE("Binding insert statement");
	hsql::printInsertStatementInfo(stmt, 1);
	auto table = BindBaseTableRef(stmt->tableName);

	if (table->GetBoundTableName().starts_with("__")) {
		throw Exception(fmt::format("invalid table for insert: {}", table->table_));
	}

	auto select = BindSelect(stmt->select);
	return std::make_unique<InsertStatement>(std::move(table), std::move(select));
}

std::unique_ptr<BoundExpressionListRef> Binder::BindValuesList(const std::vector<hsql::Expr *> &list) {
	for ([[maybe_unused]] const auto *expr : list) {
		// print
	}
	return nullptr;
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
