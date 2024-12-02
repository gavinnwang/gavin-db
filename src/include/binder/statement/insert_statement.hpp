#pragma once

#include "binder/statement/bound_statement.hpp"
#include "binder/statement/select_statement.hpp"
#include "binder/table_ref/bound_base_table_ref.hpp"
#include "binder/table_ref/bound_table_ref.hpp"
namespace db {

class InsertStatement : public BoundStatement {
public:
	explicit InsertStatement(std::unique_ptr<BoundBaseTableRef> table, std::unique_ptr<SelectStatement> select)
	    : BoundStatement(StatementType::INSERT_STATEMENT), table_(std::move(table)), select_(std::move(select)) {};

	std::string ToString() const override;

	std::unique_ptr<BoundBaseTableRef> table_;

	std::unique_ptr<SelectStatement> select_;
};

} // namespace db

template <>
struct fmt::formatter<db::InsertStatement> : formatter<std::string_view> {
	auto format(const db::InsertStatement &x, format_context &ctx) const {
		return formatter<string_view>::format(x.ToString(), ctx);
	}
};
