#pragma once

#include "common/typedef.hpp"
#include "execution/expressions/abstract_expression.hpp"
#include "fmt/core.h"
namespace db {

enum class TUPLE_POSITION { LEFT, RIGHT, INVALID };
class ColumnValueExpression : public AbstractExpression {
public:
	ColumnValueExpression(TUPLE_POSITION tuple_pos, column_t col_idx, TypeId ret_type)
	    : AbstractExpression(ret_type), tuple_pos_ {tuple_pos}, col_idx_ {col_idx} {
	}

	Value Evaluate(const Tuple &tuple, const Schema &schema) const override {
		return tuple.GetValue(schema, col_idx_);
	}

	Value GetConstValue() const override {
		UNREACHABLE("Column Value Expression cannot return a constant value");
	}

	Value EvaluateJoin(const Tuple &left_tuple, const Schema &left_schema, const Tuple &right_tuple,
	                   const Schema &right_schema) const override {
		return tuple_pos_ == TUPLE_POSITION::LEFT ? left_tuple.GetValue(left_schema, col_idx_)
		                                          : right_tuple.GetValue(right_schema, col_idx_);
	}

	auto ToString() const -> std::string override {
		return fmt::format("#{}", col_idx_);
	}

private:
	/** Tuple index 0 = left side of join, tuple index 1 = right side of join */
	// tuple_idx_;
	// Column index refers to the index within the schema of the tuple, e.g. schema {A,B,C} has indexes {0,1,2} */

	TUPLE_POSITION tuple_pos_;
	column_t col_idx_;
	std::string col_name_;
};

} // namespace db
