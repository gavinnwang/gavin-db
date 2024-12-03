
#pragma once

#include "binder/expressions/bound_expression.hpp"

#include <algorithm>
#include <iterator>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace db {

/**
 * A bound column reference, e.g., `y.x` in the SELECT list.
 */
class BoundColumnRef : public BoundExpression {
public:
	explicit BoundColumnRef(std::vector<std::string> col_name)
	    : BoundExpression(ExpressionType::COLUMN_REF), col_name_(std::move(col_name)) {
	}

	[[nodiscard]] std::string ToString() const  override {
		return fmt::format("{}", fmt::join(col_name_, "."));
	}

	/** The name of the column. */
	std::vector<std::string> col_name_;
};
} // namespace db
