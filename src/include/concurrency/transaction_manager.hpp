#pragma once

#include "common/exception.hpp"
#include "concurrency/transaction.hpp"
#include "concurrency/watermark.hpp"
namespace db {
class TransactionManager {
public:
	explicit TransactionManager() = default;
	~TransactionManager() = default;

	Transaction &Begin([[maybe_unused]] IsolationLevel isolation_level = IsolationLevel::READ_UNCOMMITTED) {
		std::unique_lock txn_map_lock(txn_map_mutex_);

		auto txn_id = next_txn_id_++;
		auto [it, inserted] = txn_map_.emplace(txn_id, std::make_unique<Transaction>(txn_id, isolation_level));
		if (!inserted) {
			throw RuntimeException("Failed to insert new transaction.");
		}
		auto &txn = it->second;
		txn->read_ts_ = last_commit_ts_.load();
		running_txns_.AddTxn(txn->read_ts_);
		return *txn;
	}

	bool Commit(Transaction &txn) {
		// std::lock_guard commit_lock {commit_mutex_};
		(void)txn;
		return true;
	}
	void Abort(Transaction &txn) {
		(void)txn;
	}

	timestamp_t GetWatermark() {
		return running_txns_.GetWatermark();
	}

private:
	/** Only one txn is allowed to commit at a time */
	std::mutex commit_mutex_;
	std::atomic<txn_id_t> next_txn_id_ {TXN_START_ID};
	/** protects txn map */
	std::shared_mutex txn_map_mutex_;
	/** All transactions, running or committed */
	std::unordered_map<txn_id_t, std::unique_ptr<Transaction>> txn_map_;
	/** The last committed timestamp. */
	std::atomic<timestamp_t> last_commit_ts_ {0};
	/** Stores all the read_ts of running txns so as to facilitate garbage collection. */
	Watermark running_txns_ {0};
};
} // namespace db
