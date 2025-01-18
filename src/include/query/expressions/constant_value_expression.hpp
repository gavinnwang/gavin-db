#pragma once

#include "query/expressions/abstract_expression.hpp"
#include "storage/table/tuple.hpp"

#include <string>
namespace db {
class ConstantValueExpression : public AbstractExpression {
public:
	explicit ConstantValueExpression(const Value &val) : AbstractExpression(val.GetTypeId()), val_(val) {
	}

	[[nodiscard]] Value Evaluate([[maybe_unused]] const Tuple &tuple, [[maybe_unused]] const Schema &schema) const override {
		return val_;
	}

	[[nodiscard]] Value GetConstValue() const override {
		return val_;
	}

	[[nodiscard]] Value EvaluateJoin([[maybe_unused]] const Tuple &left_tuple, [[maybe_unused]] const Schema &left_schema,
	                   [[maybe_unused]] const Tuple &right_tuple,
	                   [[maybe_unused]] const Schema &right_schema) const override {
		return val_;
	}

	[[nodiscard]] std::string ToString() const override {
		return val_.ToString();
	}

private:
	Value val_;
};

} // namespace db
