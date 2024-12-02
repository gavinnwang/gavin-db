#pragma once

#include "binder/expressions/bound_expression.hpp"
#include "binder/statement/bound_statement.hpp"
#include "binder/table_ref/bound_table_ref.hpp"

#include <vector>
namespace db {

class SelectStatement : public BoundStatement {
public:
	explicit SelectStatement(std::unique_ptr<BoundTableRef> table,
	                         std::vector<std::unique_ptr<BoundExpression>> select_list)
	    : BoundStatement(StatementType::SELECT_STATEMENT), table_(std::move(table)),
	      select_list_(std::move(select_list)) {
	}

	std::string ToString() const override;

	/** Bound FROM clause. */
	std::unique_ptr<BoundTableRef> table_;

	/** Bound SELECT list. */
	std::vector<std::unique_ptr<BoundExpression>> select_list_;
};

} // namespace db

template <>
struct fmt::formatter<db::SelectStatement> : fmt::formatter<std::string_view> {
	auto format(const db::SelectStatement &x, fmt::format_context &ctx) const {
		return fmt::formatter<std::string_view>::format(x.ToString(), ctx);
	}
};
