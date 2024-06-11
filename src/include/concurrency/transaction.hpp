#pragma once

#include "storage/page/page.hpp"

#include <deque>
#include <functional>
#include <memory>
#include <unordered_set>
namespace db {
enum class TransactionState { GROWING, SHRINKING, COMMITTED, ABORTED };
enum class IsolationLevel { READ_UNCOMMITTED, REPEATABLE_READ, READ_COMMITTED };
enum class WType { INSERT = 0, DELETE, UPDATE };
class Transaction {
public:
	explicit Transaction() = default;

	auto GetPageSet() {
		return page_set_;
	}
	void AddIntoPageSet(std::reference_wrapper<Page> page) {
		page_set_->push_back(page);
	}

private:
	// pages latched during index operation
	std::shared_ptr<std::deque<std::reference_wrapper<Page>>> page_set_;
	// pages deleted during index operation
	std::shared_ptr<std::unordered_set<page_id_t>> deleted_page_set_;
};
} // namespace db
