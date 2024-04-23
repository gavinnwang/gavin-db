#pragma once

#include "storage/disk.hpp"
#include "storage/page.hpp"
#include "buffer/replacer.hpp"
#include <list>
#include <memory>
namespace db{

class BufferPoolManager {
  public:

    BufferPoolManager(size_t pool_size, DiskManager *disk_manager);
    ~BufferPoolManager();
    auto NewPage(page_id_t *page_id) -> Page *;

  private:
    auto AllocatePage() -> page_id_t;

    const size_t pool_size_;
    std::atomic<page_id_t> next_page_id_ = 0;
    std::unique_ptr<Replacer> replacer_;
    std::unique_ptr<DiskManager>disk_manager_;
    std::mutex latch_;
    std::list<frame_id_t> free_list_;
    Page *pages_;

};
}
