#pragma once

#include "common/config.hpp"
#include "common/macros.hpp"
#include "common/page_id.hpp"
#include "common/rwlatch.hpp"
#include "fmt/format.h"

#include <cstring>

namespace db {
class Page {
	friend class BufferPool;

public:
	explicit Page() {
		ResetMemory();
	}

	DISALLOW_COPY_AND_MOVE(Page);

	~Page() = default;

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
	[[nodiscard]] std::string ToString() const {
		return fmt::format("Page {{ table_id={}, page_id={} is_dirty={}, pin_count={}, data_size={} }}",
		                   page_id_.table_id_, page_id_.page_number_, is_dirty_, pin_count_, data_.size());
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
};
} // namespace db

template <>
struct fmt::formatter<db::Page> : formatter<std::string_view> {
	auto format(db::Page x, format_context &ctx) const {
		return formatter<string_view>::format(x.ToString(), ctx);
	}
};
