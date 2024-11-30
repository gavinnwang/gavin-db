#pragma once

#include "binder/statement/bound_statement.hpp"
#include "binder/statement/create_statement.hpp"
#include "catalog/column.hpp"
#include "sql/CreateStatement.h"
#include "sql/SQLStatement.h"

#include <memory>

namespace db {
class Binder {
public:
	explicit Binder() {
	}
	std::unique_ptr<BoundStatement> Bind(const hsql::SQLStatement *stmt) const;
	std::unique_ptr<CreateStatement> BindCreate(const hsql::CreateStatement *stmt) const;
	Column BindColumnDefinition(const hsql::ColumnDefinition *col_def) const;

private:
};
} // namespace db
