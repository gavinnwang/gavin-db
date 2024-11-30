#pragma once

#include "binder/statement/bound_statement.hpp"
#include "binder/statement/create_statement.hpp"
#include "binder/table_ref/bound_expression_list.hpp"
#include "catalog/column.hpp"
#include "sql/CreateStatement.h"
#include "sql/SQLStatement.h"

#include <memory>

namespace db {
class Binder {
public:
	explicit Binder() {
	}
	std::unique_ptr<BoundStatement> Bind(const hsql::SQLStatement *stmt);
	std::unique_ptr<CreateStatement> BindCreate(const hsql::CreateStatement *stmt);
	// std::unique_ptr<SelectStatement> BindSelect(const hsql::SelectStatement *stmt);
	// std::unique_ptr<InsertStatement> BindInsert(const hsql::SelectStatement *stmt);
	std::unique_ptr<BoundExpressionListRef> BindValuesList(const std::vector<const hsql::Expr *> &list);
	Column BindColumnDefinition(const hsql::ColumnDefinition *col_def) const;

private:
};
} // namespace db
