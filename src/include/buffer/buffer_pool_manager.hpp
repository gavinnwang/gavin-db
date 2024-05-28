#pragma once

#include "buffer/replacer.hpp"
#include "common/config.hpp"
#include "common/macros.hpp"
#include "storage/disk_manager.hpp"
#include "storage/page.hpp"
#include "storage/page_guard.hpp"

#include <list>
#include <memory>
#include <vector>
namespace db {

class BufferPoolManager {
public:
	BufferPoolManager(size_t pool_size, DiskManager *disk_manager, page_id_t next_page_id);
	DISALLOW_COPY_AND_MOVE(BufferPoolManager);
	auto FlushPage(page_id_t page_id) -> bool;
	void FlushAllPages();
	auto FetchPageBasic(page_id_t page_id) -> BasicPageGuard;
	auto FetchPageRead(page_id_t page_id) -> ReadPageGuard;
	auto FetchPageWrite(page_id_t page_id) -> WritePageGuard;
	auto NewPageGuarded(page_id_t &page_id) -> BasicPageGuard;
	auto UnpinPage(page_id_t page_id, bool is_dirty) -> bool;

private:
	auto DeletePage(page_id_t page_id) -> bool;
	auto NewPage(page_id_t &page_id) -> Page &;
	auto FetchPage(page_id_t page_id) -> Page &;
	auto AllocatePage() -> page_id_t;
	auto AllocateFrame(frame_id_t &frame_id) -> bool;

	const frame_id_t pool_size_;
	std::atomic<page_id_t> next_page_id_;
	std::unique_ptr<Replacer> replacer_;
	std::unique_ptr<DiskManager> disk_manager_;
	std::mutex latch_;
	std::list<frame_id_t> free_list_;
	std::unordered_map<page_id_t, frame_id_t> page_table_;
	std::vector<Page> pages_;
};
} // namespace db
