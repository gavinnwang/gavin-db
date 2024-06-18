#include "execution/plans/projection_plan.hpp"
namespace db {
Schema ProjectionPlanNode::InferProjectionSchema(const std::vector<AbstractExpressionRef> &expressions) {
	std::vector<Column> schema;
	for (const auto &expr : expressions) {
		auto type_id = expr->GetReturnType();
		if (Type::IsFixedSize(type_id)) {
			schema.emplace_back("<unnamed>", type_id);
		} else {
			// schema.emplace_back("<unnamed>", type_id, VARCHAR_DEFAULT_LENGTH);
		}
	}
	return Schema(schema);
}
} // namespace db
