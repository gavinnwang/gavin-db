#pragma once

#include "binder/statement/bound_statement.hpp"
#include "binder/table_ref/bound_base_table_ref.hpp"
#include "binder/table_ref/bound_table_ref.hpp"
namespace db {

class InsertStatement : public BoundStatement {
public:
	explicit InsertStatement(std::unique_ptr<BoundBaseTableRef> table, std::unique_ptr<SelectStatement> select);

	std::unique_ptr<BoundBaseTableRef> table_;

	std::unique_ptr<SelectStatement> select_;

	auto ToString() const -> std::string override;
};

} // namespace db
