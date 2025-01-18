#pragma once

#include "query/binder/expressions/bound_expression.hpp"
#include "common/arithmetic_type.hpp"
namespace db {

/**
 * A bound binary operator, e.g., `a+b`.
 */
class BoundBinaryOp : public BoundExpression {
public:
	explicit BoundBinaryOp(ArithmeticType op, std::unique_ptr<BoundExpression> larg,
	                       std::unique_ptr<BoundExpression> rarg)
	    : BoundExpression(ExpressionType::BINARY_OP), op_(op), larg_(std::move(larg)),

	      rarg_(std::move(rarg)) {
	}

	[[nodiscard]] std::string ToString() const override {
		return fmt::format("({}{}{})", larg_, ArithmeticTypeHelper::ToString(op_), rarg_);
	}

	/** Operator name. */
	ArithmeticType op_;

	/** Left argument of the op. */
	std::unique_ptr<BoundExpression> larg_;

	/** Right argument of the op. */
	std::unique_ptr<BoundExpression> rarg_;
};
} // namespace db
