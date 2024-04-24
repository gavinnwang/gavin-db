#include "buffer/buffer_pool_manager.hpp"
#include "buffer/random_replacer.h"
#include "common/config.hpp"
#include "common/macros.hpp"
#include <memory>
#include <vector>
namespace db {
BufferPoolManager::BufferPoolManager(size_t pool_size,
                                     DiskManager *disk_manager)
    : pool_size_(pool_size),
      replacer_(std::make_unique<RandomBogoReplacer>(10)),
      disk_manager_(disk_manager) {
  pages_ = new Page[pool_size_];
  std::vector<bool> free_frame_tracker_(pool_size_, true);
  for (size_t i = 0; i < pool_size_; ++i) {
    free_list_.emplace_back(static_cast<int>(i));
  }
}
BufferPoolManager::~BufferPoolManager() { delete[] pages_; }

auto BufferPoolManager::AllocatePage() -> page_id_t { return next_page_id_++; }

auto BufferPoolManager::AllocateFrame(frame_id_t *frame_id) -> bool {
  if (free_list_.empty()) {
    // gotta evict a random frame because
    if (!replacer_->Evict(frame_id)) {
      return false;
    }
    if (pages_[*frame_id].is_dirty_) {
      auto &evict_page = pages_[*frame_id];
      disk_manager_->WritePage(evict_page.page_id_, evict_page.data_);
    }
  }
  *frame_id = free_list_.front();
  free_list_.pop_back();
  return true;
}

auto BufferPoolManager::NewPage(page_id_t *page_id) -> Page * {
  std::lock_guard<std::mutex> lock(latch_);
  frame_id_t frame_id = -1;
  if (!AllocateFrame(&frame_id)) {
    return nullptr;
  }

  ASSERT(frame_id != -1, "frame id has to be assigned a valid value here");

  replacer_->Pin(frame_id);

  // get rid fo the stale page table record
  page_table_.erase(pages_[frame_id].page_id_);

  *page_id = AllocatePage();
  page_table_[*page_id] = frame_id;
  // reset the memory and metadata for the new page
  Page *page = &pages_[frame_id];
  page->page_id_ = *page_id;
  page->pin_count_ = 1;
  page->is_dirty_ = false;
  page->ResetMemory();

  return page;
}
auto BufferPoolManager::FetchPage(page_id_t page_id) -> Page * {
  std::lock_guard<std::mutex> lock(latch_);
  if (page_table_.find(page_id) != page_table_.end()) {
    frame_id_t frame_id = page_table_[page_id];
    Page *page = &pages_[frame_id];
    page->pin_count_++;
    replacer_->Pin(frame_id);
    return page;
  }

  frame_id_t frame_id = 0;
  if (!AllocateFrame(&frame_id)) {
    return nullptr;
  }

  page_table_.erase(pages_[frame_id].page_id_);
  page_table_.insert({page_id, frame_id});

  Page *page = &pages_[frame_id];
  page->page_id_ = page_id;
  page->pin_count_++;
  page->is_dirty_ = false;
  disk_manager_->ReadPage(page_id, page->data_);

  return page;
}

auto BufferPoolManager::UnpinPage(page_id_t page_id, bool is_dirty) -> bool {
  std::lock_guard<std::mutex> lock(latch_);
  if (page_table_.find(page_id) == page_table_.end()) {
    return false;
  }
  frame_id_t frame_id = page_table_[page_id];
  Page &page = pages_[frame_id];
  if (is_dirty) {
    page.is_dirty_ = true;
  }
  if (page.pin_count_ == 0) {
    return false;
  }
  page.pin_count_--;
  if (page.pin_count_ == 0) {
    replacer_->Unpin(frame_id);
  }
  return true;
}

auto BufferPoolManager::FlushPage(page_id_t page_id) -> bool {
  std::lock_guard<std::mutex> lock(latch_);
  if (page_table_.find(page_id) == page_table_.end()) {
    return false;
  }
  frame_id_t frame_id = page_table_[page_id];
  Page &page = pages_[frame_id];
  disk_manager_->WritePage(page_id, page.data_);
  page.is_dirty_ = false;
  return true;
}

auto BufferPoolManager::DeletePage(page_id_t page_id) -> bool {
  std::lock_guard<std::mutex> lock(latch_);
  if (page_table_.find(page_id) == page_table_.end()) {
    return true;
  }
  frame_id_t frame_id = page_table_[page_id];
  Page &page = pages_[frame_id];
  if (page.pin_count_ > 0) {
    return false;
  }
  page_table_.erase(page_id);
  free_list_.push_back(frame_id);

  pages_[frame_id].ResetMemory();
  page.page_id_ = INVALID_PAGE_ID;
  page.pin_count_ = 0;
  page.is_dirty_ = false;
  return true;
}
} // namespace db