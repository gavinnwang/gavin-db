#pragma once

#include "catalog/schema.hpp"

#include <algorithm>
#include <memory>
#include <utility>
namespace db {

enum class PlanType {
	SeqScan,
	IndexScan,
	Insert,
	Update,
	Delete,
	Aggregation,
	Limit,
	NestedLoopJoin,
	NestedIndexJoin,
	HashJoin,
	Filter,
	Values,
	Projection,
	Sort,
	TopN,
	TopNPerGroup,
	MockScan,
	InitCheck,
	Window
};

class AbstractPlanNode;
using AbstractPlanNodeRef = std::shared_ptr<const AbstractPlanNode>;

class AbstractPlanNode {

public:
	AbstractPlanNode(SchemaRef output_schema, std::vector<AbstractPlanNodeRef> children)
	    : output_schema_(std::move(output_schema)), children_(std::move(children)) {
	}
	virtual ~AbstractPlanNode() = default;

	const Schema &OutputSchema() const {
		return *output_schema_;
	}

	const std::vector<AbstractPlanNodeRef> &GetChildren() const {
		return children_;
	}
	const AbstractPlanNodeRef GetChildAt(idx_t child_idx) const {
		return children_.at(child_idx);
	}
	virtual PlanType GetType() const = 0;

	virtual std::string ToString() const {
		return "<unknown>";
	}
	std::string ChildrenToString(int indent, bool with_schema = true) const;

protected:
	SchemaRef output_schema_;
	// The children of this plan node.
	std::vector<AbstractPlanNodeRef> children_;
};
} // namespace db
//
