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
    auto FetchPage(page_id_t page_id) -> Page *;
    auto UnpinPage(page_id_t page_id, bool is_dirty) -> bool;
    auto FlushPage(page_id_t page_id) -> bool;
    auto DeletePage(page_id_t page_id) -> bool;

  private:
    auto AllocatePage() -> page_id_t;
    auto AllocateFrame(frame_id_t* frame_id) -> bool;

    const size_t pool_size_;
    std::atomic<page_id_t> next_page_id_ = 0;
    std::unique_ptr<Replacer> replacer_;
    std::unique_ptr<DiskManager>disk_manager_;
    std::mutex latch_;
    std::list<frame_id_t> free_list_;
    std::unordered_map<page_id_t, frame_id_t> page_table_;
    Page *pages_;

};
}
