#include "query/executors/seq_scan_executor.hpp"

// #include <backward.hpp>
namespace db {

bool SeqScanExecutor::Next(Tuple &tuple,  RID &rid) {
	// backward::SignalHandling sh; // Automatically handles crashes
	if (table_iter_.IsEnd()) {
		LOG_TRACE("end");
		return false;
	}
	auto tuple_opt = table_iter_.GetTuple();
	if (tuple_opt.has_value()) {
		auto &[meta, t] = *tuple_opt;
		tuple = t;
		LOG_TRACE("Got tuple{}", tuple.ToString(plan_->OutputSchema()));
		rid = table_iter_.GetRID();
		++table_iter_;
	}
	return true;
}
} // namespace db
