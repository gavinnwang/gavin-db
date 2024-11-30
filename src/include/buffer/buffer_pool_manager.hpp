#pragma once

#include "buffer/replacer.hpp"
#include "common/macros.hpp"
#include "common/page_id.hpp"
#include "common/typedef.hpp"
#include "storage/disk_manager.hpp"
#include "storage/page/page.hpp"
#include "storage/page/page_guard.hpp"
#include "storage/page_allocator.hpp"

#include <list>
#include <memory>
#include <vector>
namespace db {
class BufferPoolManager {
public:
	BufferPoolManager(frame_id_t pool_size, std::shared_ptr<DiskManager> disk_manager);
	DISALLOW_COPY(BufferPoolManager);
	bool FlushPage(PageId page_id);
	void FlushAllPages();
	BasicPageGuard FetchPageBasic(PageId page_id);
	ReadPageGuard FetchPageRead(PageId page_id);
	WritePageGuard FetchPageWrite(PageId page_id);
	BasicPageGuard NewPageGuarded(PageAllocator &page_allocator, PageId &page_id);
	bool UnpinPage(PageId page_id, bool is_dirty);
	Page &NewPage(PageAllocator &page_allocator, PageId &page_id);
	Page &FetchPage(PageId page_id);
	bool DeletePage(PageId page_id);

private:
	bool AllocateFrame(frame_id_t &frame_id);
	void PrintPages() {
		for (const auto &page : pages_) {
			std::cout << "PageId: " << page.page_id_.page_number_ << " PinCount: " << page.pin_count_
			          << " Dirty: " << page.is_dirty_ << std::endl;
		}
	}
	void PrintFreeList() {
		for (const auto &frame_id : free_list_) {
			std::cout << frame_id << " ";
		}
		std::cout << std::endl;
	}

	const frame_id_t pool_size_;

	std::unique_ptr<Replacer> replacer_;
	std::shared_ptr<DiskManager> disk_manager_;
	std::list<frame_id_t> free_list_;
	std::unordered_map<PageId, frame_id_t, PageId_Hash> page_table_;
	std::vector<Page> pages_;
	std::mutex latch_;
};
} // namespace db
