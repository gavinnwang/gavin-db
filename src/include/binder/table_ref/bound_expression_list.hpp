#pragma once

#include "binder/expressions/bound_expression.hpp"
#include "binder/table_ref/bound_table_ref.hpp"
#include "fmt/ranges.h"

#include <vector>
namespace db {

/**
 * A bound table ref type for `values` clause.
 */
class BoundExpressionListRef : public BoundTableRef {
public:
	explicit BoundExpressionListRef(std::vector<std::vector<std::unique_ptr<BoundExpression>>> values)
	    : BoundTableRef(TableReferenceType::EXPRESSION_LIST), values_(std::move(values)) {
	}

	std::string ToString() const override {
		return fmt::format("BoundExpressionListRef {{ values={} }}", values_);
	}

	/** The value list */
	std::vector<std::vector<std::unique_ptr<BoundExpression>>> values_;

	/** A unique identifier for this values list, so that planner / binder can work correctly. */
};
} // namespace db

template <>
struct fmt::formatter<db::BoundExpressionListRef> : formatter<std::string_view> {
	auto format(const db::BoundExpressionListRef &x, format_context &ctx) const {
		return formatter<string_view>::format(x.ToString(), ctx);
	}
};
