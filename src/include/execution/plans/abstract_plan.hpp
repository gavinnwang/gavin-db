#pragma once

#include "catalog/schema.hpp"

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
using AbstractPlanNodeRef = std::unique_ptr<AbstractPlanNode>;

class AbstractPlanNode {
private:
	template <typename T, typename... Args>
	std::vector<T> BuildVectorFromMoveOnlyObjects(Args &&...args) {
		std::vector<T> container;
		container.reserve(sizeof...(Args));
		(container.emplace_back(std::forward<Args>(args)), ...);
		return container;
	}

public:
	explicit AbstractPlanNode(SchemaRef output_schema) : output_schema_(std::move(output_schema)) {
	}

	// AbstractPlanNode(SchemaRef output_schema, std::vector<AbstractPlanNodeRef> children)
	//     : output_schema_(std::move(output_schema)), children_(std::move(children)) {
	// }

	template <typename... Args>
	explicit AbstractPlanNode(SchemaRef output_schema, Args &&...args)
	    : output_schema_(std::move(output_schema)),
	      children_(BuildVectorFromMoveOnlyObjects<AbstractPlanNodeRef>(std::forward<Args>(args)...)) {
	}
	virtual ~AbstractPlanNode() = default;

	[[nodiscard]] const Schema &OutputSchema() const {
		return *output_schema_;
	}

	[[nodiscard]] const std::vector<AbstractPlanNodeRef> &GetChildren() const {
		return children_;
	}

	[[nodiscard]] virtual PlanType GetType() const = 0;

	[[nodiscard]] virtual std::string ToString() const {
		return "<unknown>";
	}
	[[nodiscard]] std::string ChildrenToString(int indent, bool with_schema = true) const;

protected:
	SchemaRef output_schema_;
	// The children of this plan node.
	std::vector<AbstractPlanNodeRef> children_;
};
} // namespace db
