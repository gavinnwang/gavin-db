#pragma once

#include "common/exception.hpp"
#include "common/value.hpp"
#include "execution/expressions/abstract_expression.hpp"
namespace db {

enum class ArithmeticType { Plus, Minus };
class ArithmeticExpression : public AbstractExpression {
public:
	ArithmeticExpression(AbstractExpressionRef left, AbstractExpressionRef right, ArithmeticType compute_type)
	    : AbstractExpression({std::move(left), std::move(right)}, TypeId::INTEGER), compute_type_ {compute_type} {
		if (GetChildAt(0)->GetReturnType() != TypeId::INTEGER || GetChildAt(1)->GetReturnType() != TypeId::INTEGER) {
			throw NotImplementedException("Not implemented");
		}
	}

private:
	auto PerformComputation(const Value &lhs, const Value &rhs) const -> std::optional<int32_t> {
		if (lhs.IsNull() || rhs.IsNull()) {
			return std::nullopt;
		}
		switch (compute_type_) {
		case ArithmeticType::Plus:
			return lhs.GetAs<int32_t>() + rhs.GetAs<int32_t>();
		case ArithmeticType::Minus:
			return lhs.GetAs<int32_t>() - rhs.GetAs<int32_t>();
		default:
			UNREACHABLE("Unsupported arithmetic type.");
		}
	}
	ArithmeticType compute_type_;
};

} // namespace db
