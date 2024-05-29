#pragma once

#include <shared_mutex>

namespace db {
class ReaderWriterLatch {
public:
	void WLock() {
		mutex_.lock();
	}
	void WUnlock() {
		mutex_.unlock();
	}
	void RLock() {
		mutex_.lock_shared();
	}
	void RUnlock() {
		mutex_.unlock_shared();
	}

private:
	std::shared_mutex mutex_;
};

} // namespace db
