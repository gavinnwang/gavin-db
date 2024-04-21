#pragma once

#include "storage/disk.hpp"
namespace db{
class Page {
 public:
  /** Constructor. Zeros out the page data. */
  Page() {
    data_ = new char[PAGE_SIZE];
    ResetMemory();
  }

private:
  static constexpr size_t OFFSET_PAGE_START = 0;
  inline void ResetMemory() { memset(data_, OFFSET_PAGE_START, PAGE_SIZE); }
  char * data_;
  page_id_t page_id_ = INVALID_PAGE_ID;
};
}
