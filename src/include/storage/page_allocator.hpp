#pragma once

#include "common/page_id.hpp"
#include <memory>
namespace db {
class PageAllocator {
public:
  virtual PageId AllocatePage() = 0;
    virtual ~PageAllocator() = default; // Virtual destructor
};

} // namespace db
