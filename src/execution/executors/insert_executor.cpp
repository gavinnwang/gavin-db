#include "execution/executors/insert_executor.hpp"
namespace db {

InsertExecutor::InsertExecutor(const InsertPlanNode &plan, std::unique_ptr<AbstractExecutor> &&child_executor)
    : child_executor_(std::move(child_executor)), plan_(plan) {
}
bool Next(Tuple &tuple, RID &rid) {
	(void)tuple;
	(void)rid;
	return true;
}

} // namespace db
