
#include "execution/executors/value_executor.hpp"
namespace db {

auto ValuesExecutor::Next(Tuple &tuple, [[maybe_unused]] RID &rid) -> bool {
	if (cursor_ >= plan_->GetValues().size()) {
		return false;
	}

	std::vector<Value> values {};
	values.reserve(GetOutputSchema().GetColumnCount());

	const auto &row_expr = plan_->GetValues()[cursor_];
	for (const auto &col : row_expr) {
		values.push_back(col->GetConstValue());
	}

	tuple = Tuple {values, GetOutputSchema()};
	cursor_ += 1;

	return true;
}

} // namespace db
