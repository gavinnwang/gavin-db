#pragma once

#include "catalog/catalog_manager.hpp"
#include "sql/SQLStatement.h"

namespace db {
class Binder {
public:
	explicit Binder(const std::shared_ptr<CatalogManager> &CatalogManager) : catalog_manager_(CatalogManager) {
	}
	void Parse(const std::string &query);
	std::unique_ptr<CreateStatement> BindCreate(hsql::SQLStatement hsql_stmt) -> std::unique_ptr<CreateStatement>;

private:
	const std::shared_ptr<CatalogManager> &catalog_manager_;
	std::vector<hsql::SQLStatement *> statement_nodes_;
};
} // namespace db
