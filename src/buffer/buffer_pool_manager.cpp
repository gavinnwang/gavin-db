#include "buffer/buffer_pool_manager.hpp"
#include "buffer/random_replacer.h"
#include "common/config.hpp"
#include "common/macros.hpp"
#include "storage/page_guard.hpp"
#include <iostream>
#include <memory>
#include <vector>
namespace db {
BufferPoolManager::BufferPoolManager(size_t pool_size,
                                     DiskManager *disk_manager,
                                     page_id_t next_page_id)
    : pool_size_(pool_size), next_page_id_(next_page_id),
      replacer_(std::make_unique<RandomBogoReplacer>()),
      disk_manager_(disk_manager) {
  pages_ = new Page[pool_size_];
  std::vector<bool> free_frame_tracker_(pool_size_, true);
  for (frame_id_t i = 0; i < pool_size_; ++i) {
    free_list_.emplace_back(i);
  }
}
BufferPoolManager::~BufferPoolManager() { delete[] pages_; }

auto BufferPoolManager::AllocatePage() -> page_id_t { return next_page_id_++; }

auto BufferPoolManager::AllocateFrame(frame_id_t &frame_id) -> bool {
  if (free_list_.empty()) {
    // gotta evict a random frame because
    if (!replacer_->Evict(frame_id)) {
      return false;
    }
    if (pages_[frame_id].is_dirty_) {
      auto &evict_page = pages_[frame_id];
      disk_manager_->WritePage(evict_page.page_id_, evict_page.GetData());
    }
    return true;
  }
  frame_id = free_list_.front();
  free_list_.pop_front();
  return true;
}

auto BufferPoolManager::NewPage(page_id_t &page_id) -> Page & {
  std::lock_guard<std::mutex> lock(latch_);
  frame_id_t frame_id = -1;
  if (!AllocateFrame(frame_id)) {
    throw std::runtime_error("Failed to allocate frame");
    // return nullptr;
  }
  // assert that frame id is not in the free list
  ASSERT(std::find(free_list_.begin(), free_list_.end(), frame_id) ==
             free_list_.end(),
         "frame id should not be in the free list");
  // assert that frame id is valid
  ASSERT(frame_id != -1, "frame id has to be assigned a valid value here");

  replacer_->Pin(frame_id);

  // get rid fo the stale page table record
  auto page_id_to_replace = pages_[frame_id].page_id_;
  page_table_.erase(page_id_to_replace);

  page_id = AllocatePage();
  page_table_[page_id] = frame_id;
  // reset the memory and metadata for the new page
  Page &page = pages_[frame_id];
  page.page_id_ = page_id;
  page.pin_count_ = 1;
  page.is_dirty_ = false;
  page.ResetMemory();

  return page;
}
auto BufferPoolManager::FetchPage(page_id_t page_id) -> Page & {
  std::lock_guard<std::mutex> lock(latch_);
  if (page_table_.find(page_id) != page_table_.end()) {
    frame_id_t frame_id = page_table_[page_id];
    Page &page = pages_[frame_id];
    page.pin_count_++;
    replacer_->Pin(frame_id);
    return page;
  }

  frame_id_t frame_id = 0;
  if (!AllocateFrame(frame_id)) {
    throw std::runtime_error("Failed to allocate frame");
    // return nullptr;
  }

  page_table_.erase(pages_[frame_id].page_id_);
  page_table_.insert({page_id, frame_id});

  Page &page = pages_[frame_id];
  page.page_id_ = page_id;
  page.pin_count_++;
  page.is_dirty_ = false;
  disk_manager_->ReadPage(page_id, page.GetData());

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
  disk_manager_->WritePage(page_id, page.GetData());
  page.is_dirty_ = false;
  return true;
}

void BufferPoolManager::FlushAllPages() {
  for (frame_id_t i = 0; i < pool_size_; i++) {
    FlushPage(pages_[i].page_id_);
  }
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

auto BufferPoolManager::FetchPageBasic(page_id_t page_id) -> BasicPageGuard {
  auto &page = FetchPage(page_id);
  return {*this, page};
}

auto BufferPoolManager::FetchPageRead(page_id_t page_id) -> ReadPageGuard {
  auto &page = FetchPage(page_id);
  page.RLatch();
  return {*this, page};
}

auto BufferPoolManager::FetchPageWrite(page_id_t page_id) -> WritePageGuard {
  auto &page = FetchPage(page_id);
  page.WLatch();
  return {*this, page};
}

auto BufferPoolManager::NewPageGuarded(page_id_t &page_id) -> BasicPageGuard {
  auto& page = NewPage(page_id);
  return {*this, page};
}
} // namespace db
