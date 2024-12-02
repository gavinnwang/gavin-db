#pragma once
#include "common/page_id.hpp"

#include <cstdint>
namespace db {
class RID {
public:
	RID() = default;

	RID(PageId page_id, uint32_t slot_num) : page_id_(page_id), slot_num_(slot_num) {
	}
	auto operator==(const RID &other) const -> bool {
		return page_id_ == other.page_id_ && slot_num_ == other.slot_num_;
	}
	[[nodiscard]] inline auto GetPageId() const -> PageId {
		return page_id_;
	}
	[[nodiscard]] inline auto GetSlotNum() const -> uint32_t {
		return slot_num_;
	}

	[[nodiscard]] std::string ToString() const {
		return fmt::format("RID[{}, {}]", page_id_.page_number_, slot_num_);
	}

private:
	PageId page_id_ {};
	uint32_t slot_num_ {0}; // logical offset from 0, 1...
};
} // namespace db
