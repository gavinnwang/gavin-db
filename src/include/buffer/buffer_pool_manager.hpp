#pragma once

#include "buffer/replacer.hpp"
#include "catalog/catalog_manager.hpp"
#include "common/macros.hpp"
#include "common/page_id.hpp"
#include "storage/disk_manager.hpp"
#include "storage/page.hpp"
#include "storage/page_guard.hpp"

#include <list>
#include <memory>
#include <vector>
namespace db {
class BufferPoolManager {
public:
	BufferPoolManager(size_t pool_size, std::shared_ptr<DiskManager> disk_manager,
	                  std::shared_ptr<CatalogManager> catalog_manager);
	DISALLOW_COPY(BufferPoolManager);
	bool FlushPage(PageId page_id);
	void FlushAllPages();
	BasicPageGuard FetchPageBasic(PageId page_id);
	ReadPageGuard FetchPageRead(PageId page_id);
	WritePageGuard FetchPageWrite(PageId page_id);
	BasicPageGuard NewPageGuarded(PageId &page_id);
	bool UnpinPage(PageId page_id, bool is_dirty);

private:
	bool DeletePage(PageId page_id);
	Page &NewPage(PageId &page_id);
	Page &FetchPage(PageId page_id);
	PageId AllocatePage(table_oid_t table_id);
	bool AllocateFrame(frame_id_t &frame_id);

	const frame_id_t pool_size_;

	// std::atomic<page_id_t> next_page_id_;

	std::unique_ptr<Replacer> replacer_;
	std::shared_ptr<DiskManager> disk_manager_;

	std::mutex latch_;
	std::list<frame_id_t> free_list_;
	std::unordered_map<PageId, frame_id_t, PageId_hash> page_table_;
	std::vector<Page> pages_;

	std::shared_ptr<CatalogManager> catalog_manager_;
};
} // namespace db
