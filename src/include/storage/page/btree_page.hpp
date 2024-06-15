#pragma once

#include "common/typedef.hpp"

#include <cassert>
namespace db {

enum class IndexPageType { INVALID_INDEX_PAGE = 0, HEADER_PAGE, LEAF_PAGE, INTERNAL_PAGE };

class BtreePage {
public:
	BtreePage() = delete;
	BtreePage(const BtreePage &other) = delete;
	BtreePage &operator=(const BtreePage &other) = delete;
	BtreePage(BtreePage &&other) = delete;
	BtreePage &operator=(BtreePage &&other) = delete;
	~BtreePage() = delete;

	[[nodiscard]] idx_t GetSize() const {
		return size_;
	}
	void SetSize(idx_t size) {
		size_ = size;
	}
	void IncreaseSize(idx_t amount) {
		size_ += amount;
	}

	[[nodiscard]] idx_t GetMaxSize() const {
		assert(max_size_ > 0);
		return max_size_;
	}
	void SetMaxSize(int max_size) {
		assert(max_size > 0);
		max_size_ = max_size;
	}
	[[nodiscard]] idx_t GetMinSize() const {
		if (IsLeafPage()) {
			return max_size_ / 2;
		}
		return (max_size_ + 1) / 2;
	}

	void SetPageType(IndexPageType page_type) {
		page_type_ = page_type;
	}

	[[nodiscard]] IndexPageType GetPageType() const {
		return page_type_;
	}

	[[nodiscard]] bool IsLeafPage() const {
		return page_type_ == IndexPageType::LEAF_PAGE;
	}

	[[nodiscard]] bool IsInternalPage() const {
		return page_type_ == IndexPageType::INTERNAL_PAGE;
	}
	[[nodiscard]] bool IsRootPage() const {
		return parent_page_id_ == INVALID_PAGE_ID;
	}

	[[nodiscard]] page_id_t GetParentPageId() const {
		return parent_page_id_;
	}

	void SetParentPageId(page_id_t parent_page_id) {
		parent_page_id_ = parent_page_id;
	}

	void SetPageId(page_id_t page_id) {
		page_id_ = page_id;
	}

	[[nodiscard]] page_id_t GetPageId() const {
		return page_id_;
	}

private:
	IndexPageType page_type_;
	page_id_t parent_page_id_ {INVALID_PAGE_ID};
	page_id_t page_id_;
	idx_t size_ {0};
	idx_t max_size_;
};

static constexpr int BTREE_PAGE_HEADER_SIZE = 32;
static_assert(sizeof(BtreePage) == BTREE_PAGE_HEADER_SIZE);

} // namespace db
