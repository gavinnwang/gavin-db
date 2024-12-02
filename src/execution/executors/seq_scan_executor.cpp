#include "execution/executors/seq_scan_executor.hpp"
namespace db {

bool SeqScanExecutor::Next([[maybe_unused]] Tuple &tuple, [[maybe_unused]] RID &rid) {
	return true;
}
} // namespace db
