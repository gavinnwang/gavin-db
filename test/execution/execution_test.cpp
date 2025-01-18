
#include "query/expressions/arithmetic_expression.hpp"
#include "query/expressions/constant_value_expression.hpp"

#include "gtest/gtest.h"
#include <memory>
namespace db {
TEST(ExecutionTest, ArithmeticExpressionTest) {
	AbstractExpressionRef lhs =
	    std::make_unique<ConstantValueExpression>(Value(TypeId::INTEGER, static_cast<int32_t>(9)));
	AbstractExpressionRef rhs =
	    std::make_unique<ConstantValueExpression>(Value(TypeId::INTEGER, static_cast<int32_t>(60)));

	auto ae = std::make_unique<ArithmeticExpression>(std::move(lhs), std::move(rhs), ArithmeticType::Multiply);

	auto res = ae->Evaluate(Tuple {}, Schema {});

	ASSERT_EQ(res.ToString(), "540");

	AbstractExpressionRef lhs1 =
	    std::make_unique<ConstantValueExpression>(Value(TypeId::INTEGER, static_cast<int32_t>(9)));
	AbstractExpressionRef rhs1 =
	    std::make_unique<ConstantValueExpression>(Value(TypeId::INTEGER, static_cast<int32_t>(60)));

	auto ae1 = std::make_unique<ArithmeticExpression>(std::move(lhs1), std::move(rhs1), ArithmeticType::Plus);

	auto res1 = ae1->Evaluate(Tuple {}, Schema {});

	ASSERT_EQ(res1.ToString(), "69");
}

TEST(ExecutionTest, ConstantValueExpressionTest) {
}
} // namespace db
