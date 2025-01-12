#pragma once

#include "common/exception.hpp"
#include "common/typedef.hpp"

#include <unordered_map>
namespace db {
// Track all the read timestamps
class Watermark {
public:
	explicit Watermark(timestamp_t commit_ts) : commit_ts_(commit_ts), watermark_(commit_ts) {
	}

	// Amortized O(1)
	void AddTxn(timestamp_t read_ts) {
		if (read_ts < commit_ts_) {
			throw RuntimeException("read ts < commit ts");
		}
		if (current_reads_.find(read_ts) == current_reads_.end()) {
			current_reads_[read_ts] = 0;
		}
		current_reads_[read_ts]++;
		// Probe if the watermark ts can be increased
		for (timestamp_t i = watermark_;; i++) {
			if (current_reads_.find(i) != current_reads_.end()) {
				watermark_ = i;
				break;
			}
		}
	}

	void RemoveTxn(timestamp_t read_ts) {
		current_reads_[read_ts]--;
		if (current_reads_[read_ts] == 0) {
			current_reads_.erase(read_ts);
		}
		if (current_reads_.empty()) {
			watermark_ = commit_ts_;
			return;
		}
		for (timestamp_t i = watermark_;; i++) {
			if (current_reads_.find(i) != current_reads_.end()) {
				watermark_ = i;
				break;
			}
		}
	}

	/** The caller should update commit ts before removing the txn from the watermark so that we can track watermark
	 * correctly. */
	void UpdateCommitTs(timestamp_t commit_ts) {
		commit_ts_ = commit_ts;
	}

	timestamp_t GetWatermark() {
		if (current_reads_.empty()) {
			return commit_ts_;
		}
		return watermark_;
	}

	timestamp_t commit_ts_;

	timestamp_t watermark_;

	std::unordered_map<timestamp_t, int> current_reads_;
};

}; // namespace db
