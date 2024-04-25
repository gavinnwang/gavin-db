#pragma once
#include "common/config.hpp"
#include <cstdint>
namespace db {
class RID {
public:
  RID() = default;

  RID(page_id_t page_id, uint32_t slot_num)
      : page_id_(page_id), slot_num_(slot_num) {}
  auto operator==(const RID &other) const -> bool {
    return page_id_ == other.page_id_ && slot_num_ == other.slot_num_;
  }
  inline auto GetPageId() const -> page_id_t { return page_id_; }
  inline auto GetSlotNum() const -> uint32_t { return slot_num_; }

private:
  page_id_t page_id_{INVALID_PAGE_ID};
  uint32_t slot_num_{0}; // logical offset from 0, 1...
};
} // namespace db
