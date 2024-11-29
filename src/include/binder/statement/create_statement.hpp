#pragma once
#include "binder/statement/bound_statement.hpp"
#include "catalog/column.hpp"
namespace db {

class CreateStatement : public BoundStatement {
public:
	explicit CreateStatement(std::string table, std::vector<Column> columns, std::vector<std::string> primary_key);

	std::string table_;
	std::vector<Column> columns_;
	std::vector<std::string> primary_key_;

	auto ToString() const -> std::string override;
};
} // namespace db
