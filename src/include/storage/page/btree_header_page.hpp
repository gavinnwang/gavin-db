#pragma once

#include "common/typedef.hpp"
#include "storage/page/btree_page.hpp"
namespace db {

class BtreeHeaderPage : public BtreePage {
public:
	BtreeHeaderPage() = delete;
	BtreeHeaderPage(const BtreeHeaderPage &other) = delete;
	BtreeHeaderPage &operator=(const BtreeHeaderPage &other) = delete;
	~BtreeHeaderPage() = delete;
	BtreeHeaderPage(BtreeHeaderPage &&other) = delete;
	BtreeHeaderPage &operator=(BtreeHeaderPage &&other) = delete;

	void Init() {
		root_page_id_ = INVALID_PAGE_ID;
		SetPageType(IndexPageType::HEADER_PAGE);
	}

	void SetRootPageId(const db::page_id_t &root_page_id) {
		root_page_id_ = root_page_id;
	}

	[[nodiscard]] page_id_t GetRootPageId() const {
		return root_page_id_;
	}

	[[nodiscard]] bool TreeIsEmpty() const {
		return root_page_id_ == INVALID_PAGE_ID;
	}

private:
	page_id_t root_page_id_ {INVALID_PAGE_ID};
};

} // namespace db
