#pragma once

#include "binder/expressions/bound_expression.hpp"
#include "binder/table_ref/bound_table_ref.hpp"
namespace db {

/**
 * A bound table ref type for `values` clause.
 */
class BoundExpressionListRef : public BoundTableRef {
public:
	explicit BoundExpressionListRef(std::vector<std::vector<std::unique_ptr<BoundExpression>>> values,
	                                std::string identifier)
	    : BoundTableRef(TableReferenceType::EXPRESSION_LIST), values_(std::move(values)),
	      identifier_(std::move(identifier)) {
	}

	auto ToString() const -> std::string override;

	/** The value list */
	std::vector<std::vector<std::unique_ptr<BoundExpression>>> values_;

	/** A unique identifier for this values list, so that planner / binder can work correctly. */
	std::string identifier_;
};
} // namespace db
