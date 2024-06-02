#include "buffer/buffer_pool_manager.hpp"

#include "buffer/random_replacer.h"
#include "common/config.hpp"
#include "common/macros.hpp"
#include "storage/page_guard.hpp"

#include <memory>
#include <vector>
namespace db {
BufferPoolManager::BufferPoolManager(size_t pool_size, std::shared_ptr<DiskManager> disk_manager,
                                     std::shared_ptr<CatalogManager> catalog_manager)
    : pool_size_(pool_size), replacer_(std::make_unique<RandomBogoReplacer>()), disk_manager_(std::move(disk_manager)),
      pages_(pool_size), catalog_manager_(std::move(catalog_manager)) {
	for (frame_id_t i = 0; i < pool_size_; ++i) {
		free_list_.emplace_back(i);
	}
}

PageId BufferPoolManager::AllocatePage(table_oid_t table_oid) {
  auto new_page_num = catalog_manager_->GetLastPageId(table_oid) + 1;
	return {table_oid, new_page_num};
}

bool BufferPoolManager::AllocateFrame(frame_id_t &frame_id) {
	if (free_list_.empty()) {
		// gotta evict a random frame because
		if (!replacer_->Evict(frame_id)) {
			return false;
		}
		if (pages_[frame_id].is_dirty_) {
			auto &evict_page = pages_[frame_id];
			disk_manager_->WritePage(evict_page.GetPageId(), evict_page.GetData());
		}
		return true;
	}
	frame_id = free_list_.front();
	free_list_.pop_front();
	return true;
}

Page &BufferPoolManager::NewPage(PageId &page_id) {
	std::lock_guard<std::mutex> lock(latch_);
	frame_id_t frame_id = -1;
	if (!AllocateFrame(frame_id)) {
		throw std::runtime_error("Failed to allocate frame");
		// return nullptr;
	}
	// assert that frame id is not in the free list
	ASSERT(std::find(free_list_.begin(), free_list_.end(), frame_id) == free_list_.end(),
	       "frame id should not be in the free list");
	// assert that frame id is valid
	ASSERT(frame_id != -1, "frame id has to be assigned a valid value here");

	replacer_->Pin(frame_id);

	// get rid fo the stale page table record
	auto page_id_to_replace = pages_[frame_id].page_id_;
	page_table_.erase(page_id_to_replace);

	// assign passed in page_id to the new page
	page_id = AllocatePage(page_id.table_id_);
	page_table_[page_id] = frame_id;
  ASSERT(page_table_.at(page_id) == frame_id, "page table should have the new page id");
  ASSERT(page_table_.contains(page_id_to_replace) == false, "page table should not have the old page id");
	// reset the memory and metadata for the new page
	Page &page = pages_[frame_id];
	page.page_id_ = page_id;
	page.pin_count_ = 1;
	page.is_dirty_ = false;
	page.ResetMemory();

	return page;
}

Page &BufferPoolManager::FetchPage(PageId page_id) {
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

bool BufferPoolManager::UnpinPage(PageId page_id, bool is_dirty) {
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

bool BufferPoolManager::FlushPage(PageId page_id) {
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

bool BufferPoolManager::DeletePage(PageId page_id) {
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
	page.page_id_.page_number_ = INVALID_PAGE_ID;
	page.pin_count_ = 0;
	page.is_dirty_ = false;
	return true;
}

BasicPageGuard BufferPoolManager::FetchPageBasic(PageId page_id) {
	auto &page = FetchPage(page_id);
	return {*this, page};
}

ReadPageGuard BufferPoolManager::FetchPageRead(PageId page_id) {
	auto &page = FetchPage(page_id);
	page.RLatch();
	return {*this, page};
}

WritePageGuard BufferPoolManager::FetchPageWrite(PageId page_id) {
	auto &page = FetchPage(page_id);
	page.WLatch();
	return {*this, page};
}

BasicPageGuard BufferPoolManager::NewPageGuarded(PageId &page_id) {
	auto &page = NewPage(page_id);
	return {*this, page};
}
} // namespace db
