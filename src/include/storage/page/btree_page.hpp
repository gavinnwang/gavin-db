#pragma once

#include "common/typedef.hpp"
namespace db {

enum class IndexPageType { INVALID_INDEX_PAGE = 0, LEAF_PAGE, INTERNAL_PAGE };

class BtreePage {
public:
	BtreePage() = delete;
	BtreePage(const BtreePage &other) = delete;
	BtreePage &operator=(const BtreePage &other) = delete;
	BtreePage(BtreePage &&other) = delete;
	BtreePage &operator=(BtreePage &&other) = delete;
	~BtreePage() = delete;

	idx_t GetSize() const {
		return size_;
	}
	void SetSize(int size) {
		size_ = size;
	}
	void IncreaseSize(int amount) {
		size_ += amount;
	}

	idx_t GetMaxSize() const {
		return max_size_;
	}
	void SetMaxSize(int max_size) {
		max_size_ = max_size;
	}
	idx_t GetMinSize() const {
		return max_size_ / 2 - 1;
	}

	void SetPageType(IndexPageType page_type) {
		page_type_ = page_type;
	}

	bool IsLeafPage() const {
		return page_type_ == IndexPageType::LEAF_PAGE;
	}

	bool IsInternalPage() const {
		return page_type_ == IndexPageType::INTERNAL_PAGE;
	}
	bool IsRootPage() const {
		return parent_page_id_ == INVALID_PAGE_ID;
	}

	page_id_t GetParentPageId() const {
		return parent_page_id_;
	}

	void SetParentPageId(page_id_t parent_page_id) {
		parent_page_id_ = parent_page_id;
	}

private:
	IndexPageType page_type_;
	page_id_t parent_page_id_ {INVALID_PAGE_ID};
	idx_t size_ {0};
	idx_t max_size_;
};

} // namespace db
