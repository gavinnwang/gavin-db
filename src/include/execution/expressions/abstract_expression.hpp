#pragma once

#include "common/type.hpp"
#include "common/typedef.hpp"

#include <memory>
#include <vector>
namespace db {

class AbstractExpression;
using AbstractExpressionRef = std::shared_ptr<AbstractExpression>;

class AbstractExpression {
public:
	AbstractExpression(std::vector<AbstractExpressionRef> children, TypeId ret_type)
	    : children_ {std::move(children)}, ret_type_ {ret_type} {
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

	virtual std::string ToString() const {
		return "<unknown>";
	}

protected:
	std::vector<AbstractExpressionRef> children_;

private:
	// return type of the expression
	TypeId ret_type_;
};

} // namespace db
