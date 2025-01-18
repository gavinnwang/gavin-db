#pragma once

#include "common/exception.hpp"
#include "common/typedef.hpp"
#include "query/expressions/abstract_expression.hpp"
#include "fmt/core.h"
namespace db {

enum class TuplePosition { LEFT, RIGHT, INVALID };
class ColumnValueExpression : public AbstractExpression {
public:
	ColumnValueExpression(TuplePosition tuple_pos, column_t col_idx, TypeId ret_type)
	    : AbstractExpression(ret_type), tuple_pos_ {tuple_pos}, col_idx_ {col_idx} {
	}

	[[nodiscard]] Value Evaluate(const Tuple &tuple, const Schema &schema) const override {
		return tuple.GetValue(schema, col_idx_);
	}

	[[nodiscard]] Value GetConstValue() const override {
		throw RuntimeException("Column Value Expression cannot return a constant value");
	}

	[[nodiscard]] Value EvaluateJoin(const Tuple &left_tuple, const Schema &left_schema, const Tuple &right_tuple,
	                   const Schema &right_schema) const override {
		return tuple_pos_ == TuplePosition::LEFT ? left_tuple.GetValue(left_schema, col_idx_)
		                                          : right_tuple.GetValue(right_schema, col_idx_);
	}

	[[nodiscard]] std::string ToString() const  override {
		return fmt::format("#{}", col_idx_);
	}

private:
	/** Tuple index 0 = left side of join, tuple index 1 = right side of join */
	// tuple_idx_;
	// Column index refers to the index within the schema of the tuple, e.g. schema {A,B,C} has indexes {0,1,2} */

	TuplePosition tuple_pos_;
	column_t col_idx_;
	std::string col_name_;
};

} // namespace db
