#pragma once

#include "binder/expressions/bound_expression.hpp"
#include "binder/statement/bound_statement.hpp"
#include "binder/table_ref/bound_table_ref.hpp"
namespace db {

class SelectStatement : public BoundStatement {

	explicit SelectStatement(std::unique_ptr<BoundTableRef> table,
	                         std::vector<std::unique_ptr<BoundExpression>> select_list)
	    : BoundStatement(StatementType::SELECT_STATEMENT), table_(std::move(table)),
	      select_list_(std::move(select_list)) {
	}
	/** Bound FROM clause. */
	std::unique_ptr<BoundTableRef> table_;

	/** Bound SELECT list. */
	std::vector<std::unique_ptr<BoundExpression>> select_list_;
	std::string ToString() const override;
};
} // namespace db
