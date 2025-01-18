#pragma once

#include "query/binder/expressions/bound_expression.hpp"
namespace db {

/**
 * The star in SELECT list, e.g. `SELECT * FROM x`.
 */
class BoundStar : public BoundExpression {
public:
	BoundStar() : BoundExpression(ExpressionType::STAR) {
	}
	[[nodiscard]] std::string ToString() const override {
		return "*";
	}
};
} // namespace db
