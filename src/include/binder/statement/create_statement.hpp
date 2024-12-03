#pragma once
#include "binder/statement/bound_statement.hpp"
#include "catalog/column.hpp"
namespace db {

class CreateStatement : public BoundStatement {
public:
	explicit CreateStatement(std::string table, std::vector<Column> columns, std::vector<std::string> primary_key);

	std::string table_name_;
	std::vector<Column> columns_;
	std::vector<std::string> primary_key_;

	[[nodiscard]] std::string ToString() const   override;
};
} // namespace db
