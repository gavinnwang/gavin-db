#pragma once

#include "storage/disk.hpp"
#include "storage/page.hpp"
#include "buffer/replacer.hpp"
#include <list>
#include <memory>
namespace db{
class RandomBogoReplacer : public Replacer {
  public: 
    explicit RandomBogoReplacer(size_t num_frames);
    ~RandomBogoReplacer();
    auto Evict(frame_id_t *frame_id) -> bool;
    void Pin(frame_id_t frame_id);
    void Unpin(frame_id_t frame_id);
  private:
       
};

class BufferPoolManager {
  public:

    BufferPoolManager(size_t pool_size, DiskManager *disk_manager);
    ~BufferPoolManager();
    auto NewPage(page_id_t *page_id) -> Page *;

  private:
    const size_t pool_size_;
    std::atomic<page_id_t> next_page_id_ = 0;

    // array of pages in the buffer pool
    Page *pages_;

    std::unique_ptr<Replacer> replacer_;
    std::unique_ptr<DiskManager>disk_manager_;
    std::list<frame_id_t> free_list_;
    std::mutex latch_;

    auto AllocatePage() -> page_id_t;
};
}
