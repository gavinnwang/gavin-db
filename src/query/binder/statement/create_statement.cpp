
#include "query/binder/statement/create_statement.hpp"

#include "fmt/ranges.h"

#include <string>
namespace db {

CreateStatement::CreateStatement(std::string table, std::vector<Column> columns, std::vector<std::string> primary_key)
    : BoundStatement(StatementType::CREATE_STATEMENT), table_name_(std::move(table)), columns_(std::move(columns)),
      primary_key_(std::move(primary_key)) {
}

auto CreateStatement::ToString() const -> std::string {
	return fmt::format("BoundCreate {{\n  table={}\n  columns={}\n  primary_key={}\n}}", table_name_, columns_,
	                   primary_key_);
}

} // namespace db
