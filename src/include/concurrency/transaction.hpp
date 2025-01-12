#pragma once

#include "common/logger.hpp"
#include "common/typedef.hpp"
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
	explicit Transaction(txn_id_t txn_id, IsolationLevel isolation_level)
	    : txn_id_(txn_id), isolation_level_(isolation_level),
	      page_set_(std::make_shared<std::deque<std::reference_wrapper<Page>>>()),
	      deleted_page_set_(std::make_shared<std::unordered_set<page_id_t>>()) {
	}

	auto GetPageSet() {
		return page_set_;
	}

	void AddIntoPageSet(std::reference_wrapper<Page> page) {
		LOG_TRACE("Adding page {} into page set", page.get().GetPageId().page_number_);
		page_set_->push_back(page);
	}

private:
	txn_id_t txn_id_;
	IsolationLevel isolation_level_;
	// pages latched during index operation
	std::shared_ptr<std::deque<std::reference_wrapper<Page>>> page_set_;
	// pages deleted during index operation
	std::shared_ptr<std::unordered_set<page_id_t>> deleted_page_set_;
};
} // namespace db
