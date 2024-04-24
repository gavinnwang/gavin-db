#pragma once

#include "buffer/replacer.hpp"
#include "common/config.hpp"
#include "storage/disk.hpp"
#include "storage/page.hpp"
#include "storage/page_guard.hpp"
#include <list>
#include <memory>
namespace db {

class BufferPoolManager {
public:
  BufferPoolManager(size_t pool_size, DiskManager *disk_manager);
  ~BufferPoolManager();
  auto NewPage(page_id_t *page_id) -> Page *;
  auto FetchPage(page_id_t page_id) -> Page *;
  auto UnpinPage(page_id_t page_id, bool is_dirty) -> bool;
  auto FlushPage(page_id_t page_id) -> bool;
  void FlushAllPages();
  auto DeletePage(page_id_t page_id) -> bool;
  auto FetchPageBasic(page_id_t page_id) -> BasicPageGuard;
  auto FetchPageRead(page_id_t page_id) -> ReadPageGuard;
  auto FetchPageWrite(page_id_t page_id) -> WritePageGuard;
  auto NewPageGuarded(page_id_t *page_id) -> BasicPageGuard;

private:
  auto AllocatePage() -> page_id_t;
  auto AllocateFrame(frame_id_t *frame_id) -> bool;

  const frame_id_t pool_size_;
  std::atomic<page_id_t> next_page_id_ = 0;
  std::unique_ptr<Replacer> replacer_;
  std::unique_ptr<DiskManager> disk_manager_;
  std::mutex latch_;
  std::list<frame_id_t> free_list_;
  std::unordered_map<page_id_t, frame_id_t> page_table_;
  Page *pages_;
};
} // namespace db
