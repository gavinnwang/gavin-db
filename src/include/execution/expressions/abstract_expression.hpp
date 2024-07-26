#pragma once

#include "common/type.hpp"
#include "common/typedef.hpp"
#include "common/value.hpp"
#include "storage/table/tuple.hpp"

#include <memory>
#include <vector>
namespace db {

class AbstractExpression;
using AbstractExpressionRef = std::unique_ptr<AbstractExpression>;

class AbstractExpression {

private:
	template <typename T, typename... Args>
	std::vector<T> BuildVectorFromMoveOnlyObjects(Args &&...args) {
		std::vector<T> container;
		container.reserve(sizeof...(Args));
		(container.emplace_back(std::forward<Args>(args)), ...);
		return container;
	}

public:
	AbstractExpression(TypeId ret_type) : ret_type_ {ret_type} {
	}

	AbstractExpression(std::vector<AbstractExpressionRef> children, TypeId ret_type)
	    : children_ {std::move(children)}, ret_type_ {ret_type} {
	}

	template <typename... Args>
	AbstractExpression(TypeId ret_type, Args &&...args)
	    : children_(BuildVectorFromMoveOnlyObjects<AbstractExpressionRef>(std::forward<Args>(args)...)),
	      ret_type_ {ret_type} {
	}

	virtual ~AbstractExpression() = default;

	const AbstractExpressionRef &GetChildAt(idx_t child_idx) const {
		return children_.at(child_idx);
	}

	const std::vector<AbstractExpressionRef> &GetChildren() const {
		return children_;
	}

	TypeId GetReturnType() const {
		return ret_type_;
	}

	virtual Value Evaluate(const Tuple &tuple, const Schema &schema) const = 0;

	virtual Value GetConstValue() const = 0;

	virtual Value EvaluateJoin(const Tuple &left_tuple, const Schema &left_schema, const Tuple &right_tuple,
	                           const Schema &right_schema) const = 0;

	virtual std::string ToString() const = 0;

protected:
	std::vector<AbstractExpressionRef> children_;

private:
	// return type of the expression
	TypeId ret_type_;
	// uint32_t variable_return_size_;
};

} // namespace db
