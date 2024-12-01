
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

	std::unique_ptr<BoundColumnRef> Prepend(std::unique_ptr<BoundColumnRef> self, std::string prefix) {
		if (self == nullptr) {
			return nullptr;
		}
		std::vector<std::string> col_name {std::move(prefix)};
		std::copy(self->col_name_.cbegin(), self->col_name_.cend(), std::back_inserter(col_name));
		return std::make_unique<BoundColumnRef>(std::move(col_name));
	}

	auto ToString() const -> std::string override {
		return fmt::format("{}", fmt::join(col_name_, "."));
	}

	/** The name of the column. */
	std::vector<std::string> col_name_;
};
} // namespace db
