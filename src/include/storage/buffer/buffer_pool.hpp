#pragma once

#include "common/page_id.hpp"
#include "common/typedef.hpp"
#include "storage/buffer/replacer.hpp"
#include "storage/disk_manager.hpp"
#include "storage/page/page.hpp"
#include "storage/page/page_guard.hpp"
#include "storage/page_allocator.hpp"

#include <list>
#include <memory>
#include <vector>
namespace db {
class BufferPool {
public:
	BufferPool(frame_id_t pool_size, DiskManager &disk_manager);
	BufferPool(const BufferPool &) = delete;
	BufferPool &operator=(const BufferPool &) = delete;
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

	const frame_id_t pool_size_;
	std::unique_ptr<Replacer> replacer_;
	DiskManager &disk_manager_;
	std::list<frame_id_t> free_list_;
	std::unordered_map<PageId, frame_id_t, PageIdHash> page_table_;
	std::vector<Page> pages_;
	std::mutex latch_;
};
} // namespace db
