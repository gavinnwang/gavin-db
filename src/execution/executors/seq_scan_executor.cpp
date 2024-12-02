#include "execution/executors/seq_scan_executor.hpp"
namespace db {

bool SeqScanExecutor::Next([[maybe_unused]] Tuple &tuple, [[maybe_unused]] RID &rid) {
	LOG_TRACE("hi");
	if (table_iter_.IsEnd()) {
		LOG_TRACE("end");
		return false;
	}
	auto tuple_opt = table_iter_.GetTuple();
	LOG_TRACE("got");
	if (tuple_opt.has_value()) {
		auto &[meta, t] = *tuple_opt;
		tuple = t;
		LOG_TRACE("Got tuple{}", tuple.ToString(plan_->OutputSchema()));
		rid = table_iter_.GetRID();
		++table_iter_;
	}
	LOG_TRACE("return");
	return true;
}
} // namespace db
