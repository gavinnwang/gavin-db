#include "binder/binder.hpp"

#include "SQLParser.h"
#include "common/exception.hpp"
#include "sql/CreateStatement.h"
#include "sql/SQLStatement.h"

#include <memory>

namespace db {
std::unique_ptr<BoundStatement> Binder::Bind(const std::unique_ptr<const hsql::SQLStatement> stmt) {
	switch (stmt->type()) {
	case hsql::kStmtCreate: {
		auto rawCreateStmt = static_cast<const hsql::CreateStatement *>(stmt.get());
		std::unique_ptr<const hsql::CreateStatement> create_stmt(rawCreateStmt);
		return BindCreate(std::move(create_stmt));
	}
	default:
		throw NotImplementedException("This statement is not supported");
	}
}

std::unique_ptr<CreateStatement> Binder::BindCreate(const std::unique_ptr<const hsql::CreateStatement> stmt) {
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
				throw NotImplementedException("cannot have two primary keys");
			}
			columns.emplace_back(col);
		}
	} else {
		throw NotImplementedException("unsupported create statement");
	}
	return std::make_unique<CreateStatement>(std::move(table), std::move(columns), std::move(primary_key));
}

Column Binder::BindColumnDefinition(const hsql::ColumnDefinition *col_def) {
	auto col_type = col_def->type;
	std::string col_name = std::string(col_def->name);
	return Column {std::move(col_name), Type::HsqlColumnTypeToTypeId(col_type)};
}

} // namespace db