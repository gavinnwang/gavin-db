#pragma once

#include "common/config.hpp"
#include "common/macros.hpp"
#include "common/page_id.hpp"
#include "common/rwlatch.hpp"

#include <cstring>

namespace db {
class Page {
	friend class BufferPoolManager;

public:
	explicit Page() {
		ResetMemory();
	}
	// Page(Page &&other) noexcept;            // Move constructor
	// Page &operator=(Page &&other) noexcept; // Move assignment operator

	DISALLOW_COPY_AND_MOVE(Page);

	~Page() = default;
	// ~Page() = delete;

	auto GetData() -> char * {
		return data_.data();
	}
	auto GetPageId() -> PageId {
		return page_id_;
	}

	template <class T>
	auto As() -> const T & {
		return reinterpret_cast<const T &>(*GetData());
	}

	auto GetDataMut() -> char * {
		return GetData();
	}

	template <class T>
	auto AsMut() -> T & {
		return reinterpret_cast<T &>(*GetDataMut());
	}
	void WLatch() {
		rwlatch_.WLock();
	}
	void WUnlatch() {
		rwlatch_.WUnlock();
	}
	void RLatch() {
		rwlatch_.RLock();
	}
	void RUnlatch() {
		rwlatch_.RUnlock();
	}

private:
	void ResetMemory() {
		std::memset(data_.data(), 0, PAGE_SIZE);
	}
	PageId page_id_;
	bool is_dirty_ = false;
	uint16_t pin_count_ = 0;
	ReaderWriterLatch rwlatch_;
	std::array<char, PAGE_SIZE> data_ {};
	// char data_[PAGE_SIZE] {};
};
} // namespace db
