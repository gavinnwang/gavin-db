#pragma once

#include "common/exception.hpp"
#include "concurrency/transaction.hpp"
namespace db {
class TransactionManager {
public:
	explicit TransactionManager() = default;
	~TransactionManager() = default;

	Transaction &Begin([[maybe_unused]] IsolationLevel isolation_level) {
		std::unique_lock txn_map_lock(txn_map_mutex_);

		auto txn_id = next_txn_id_++;
		auto txn = std::make_unique<Transaction>(txn_id, isolation_level);
		auto [it, inserted] = txn_map_.emplace(txn_id, std::make_unique<Transaction>(txn_id, isolation_level));
		if (!inserted) {
			throw RuntimeException("Failed to insert new transaction.");
		}

		return *it->second;
	}

	bool Commit(Transaction *txn) {
		std::lock_guard commit_lock {commit_mutex_};
	}

private:
	/** Only one txn is allowed to commit at a time */
	std::mutex commit_mutex_;
	std::atomic<txn_id_t> next_txn_id_ {TXN_START_ID};
	/** protects txn map */
	std::shared_mutex txn_map_mutex_;
	/** All transactions, running or committed */
	std::unordered_map<txn_id_t, std::unique_ptr<Transaction>> txn_map_;
};
} // namespace db
