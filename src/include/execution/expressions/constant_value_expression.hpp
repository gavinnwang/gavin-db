#pragma once

#include "execution/expressions/abstract_expression.hpp"

#include <string>
namespace db {
class ConstantValueExpression : public AbstractExpression {
public:
	explicit ConstantValueExpression(const Value &val) : AbstractExpression(val.GetTypeId()), val_(val) {
	}

	Value Evaluate([[maybe_unused]] const Tuple &tuple, [[maybe_unused]] const Schema &schema) const override {
		return val_;
	}

	std::string ToString() const override {
		return val_.ToString();
	}

private:
	Value val_;
};

} // namespace db
