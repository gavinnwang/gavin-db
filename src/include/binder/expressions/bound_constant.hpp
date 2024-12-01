#pragma once

#include "binder/expressions/bound_expression.hpp"
#include "common/value.hpp"

#include <string>
#include <utility>

namespace db {

class BoundExpression;

/**
 * A bound constant, e.g., `1`.
 */
class BoundConstant : public BoundExpression {
public:
	explicit BoundConstant(const Value &val) : BoundExpression(ExpressionType::CONSTANT), val_(val) {
	}

	std::string ToString() const override {
		return val_.ToString();
	}

	/** The constant being bound. */
	Value val_;
};
} // namespace db
