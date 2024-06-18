#pragma once

#include "common/arithmetic_type.hpp"
#include "common/exception.hpp"
#include "common/value.hpp"
#include "execution/expressions/abstract_expression.hpp"
#include "fmt/core.h"
#include "storage/table/tuple.hpp"
namespace db {

class ArithmeticExpression : public AbstractExpression {
public:
	ArithmeticExpression(AbstractExpressionRef left, AbstractExpressionRef right, ArithmeticType compute_type)
	    : AbstractExpression {TypeId::INTEGER, std::move(left), std::move(right)}, compute_type_ {compute_type} {
		if (GetChildAt(0)->GetReturnType() != TypeId::INTEGER || GetChildAt(1)->GetReturnType() != TypeId::INTEGER) {
			throw NotImplementedException("Not implemented");
		}
	}

	Value Evaluate(const Tuple &tuple, const Schema &schema) const override {
		Value lhs = GetChildAt(0)->Evaluate(tuple, schema);
		Value rhs = GetChildAt(1)->Evaluate(tuple, schema);
		return PerformComputation(std::move(lhs), std::move(rhs));
	}
	Value EvaluateJoin(const Tuple &left_tuple, const Schema &left_schema, const Tuple &right_tuple,
	                   const Schema &right_schema) const override {
		Value lhs = GetChildAt(0)->EvaluateJoin(left_tuple, left_schema, right_tuple, right_schema);
		Value rhs = GetChildAt(1)->EvaluateJoin(left_tuple, left_schema, right_tuple, right_schema);
		return PerformComputation(std::move(lhs), std::move(rhs));
	}

	std::string ToString() const override {
		return fmt::format("{} {} {}", GetChildAt(0)->ToString(), ArithmeticTypeHelper::ToString(compute_type_),
		                   GetChildAt(1)->ToString());
	}

private:
	Value PerformComputation(Value &&lhs, Value &&rhs) const {
		if (lhs.IsNull() || rhs.IsNull()) {
			return lhs;
		}
		lhs.ComputeArithmetic(rhs, compute_type_);
		return lhs;
	}
	ArithmeticType compute_type_;
};

} // namespace db
