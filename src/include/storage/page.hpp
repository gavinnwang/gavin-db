#pragma once

#include "common/config.hpp"
#include "common/rwlatch.hpp"

namespace db {
class Page {
  friend class BufferPoolManager;

public:
  /** Constructor. Zeros out the page data. */
  explicit Page() : data_(PAGE_SIZE, 0) {}
  Page(Page &&other) noexcept;            // Move constructor
  Page &operator=(Page &&other) noexcept; // Move assignment operator
  // ~Page() { delete[] data_; }

  inline auto GetData() -> char * { return data_.data(); }
  inline auto GetPageId() -> page_id_t { return page_id_; }

  template <class T> auto As() -> const T * {
    return reinterpret_cast<const T *>(GetData());
  }

  auto GetDataMut() -> char * { return GetData(); }

  template <class T> auto AsMut() -> T * {
    return reinterpret_cast<T *>(GetDataMut());
  }
  inline void WLatch() { rwlatch_.WLock(); }
  inline void WUnlatch() { rwlatch_.WUnlock(); }
  inline void RLatch() { rwlatch_.RLock(); }
  inline void RUnlatch() { rwlatch_.RUnlock(); }

private:
  inline void ResetMemory() { std::fill(data_.begin(), data_.end(), 0); }
  page_id_t page_id_ = INVALID_PAGE_ID;
  bool is_dirty_ = false;
  uint16_t pin_count_ = 0;
  ReaderWriterLatch rwlatch_;

  // char *data_;
  std::vector<char> data_;
};
} // namespace db
