#pragma once

#include "common/rwlatch.hpp"
#include "storage/disk.hpp"
namespace db {
class Page {
  friend class BufferPoolManager;

public:
  /** Constructor. Zeros out the page data. */
  Page() {
    data_ = new char[PAGE_SIZE];
    ResetMemory();
  }
  ~Page() { delete[] data_; }

  inline auto GetData() -> char * { return data_; }
  inline auto GetPageId() -> page_id_t { return page_id_; }

  template <class T> auto As() -> const T * {
    return reinterpret_cast<const T *>(GetData());
  }

  auto GetDataMut() -> char * { return GetData(); }

  template <class T> auto AsMut() -> T * {
    return reinterpret_cast<T *>(GetDataMut());
  }

private:
  static constexpr size_t OFFSET_PAGE_START = 0;
  inline void ResetMemory() { memset(data_, OFFSET_PAGE_START, PAGE_SIZE); }
  page_id_t page_id_ = INVALID_PAGE_ID;
  bool is_dirty_ = false;
  uint16_t pin_count_ = 0;
  ReaderWriterLatch rwlatch_;

  char *data_;
};
} // namespace db
