#include "binder/binder.hpp"

#include "common/exception.hpp"
#include "common/logger.hpp"
#include "sql/ColumnType.h"
#include "sql/CreateStatement.h"
#include "sql/SQLStatement.h"

#include <memory>

namespace db {
std::unique_ptr<BoundStatement> Binder::Bind(const hsql::SQLStatement *stmt) const {
	switch (stmt->type()) {
	case hsql::kStmtCreate: {
		const auto *create_stmt = static_cast<const hsql::CreateStatement *>(stmt);
		return BindCreate(create_stmt);
	}
	default:
		throw NotImplementedException("This statement is not supported");
	}
}

std::unique_ptr<CreateStatement> Binder::BindCreate(const hsql::CreateStatement *stmt) const {
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
