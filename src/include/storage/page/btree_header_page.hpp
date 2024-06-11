#pragma once

#include "common/typedef.hpp"
namespace db {

class BtreeHeaderPage {
public:
	BtreeHeaderPage() = delete;
	BtreeHeaderPage(const BtreeHeaderPage &other) = delete;
	BtreeHeaderPage &operator=(const BtreeHeaderPage &other) = delete;
	~BtreeHeaderPage() = delete;
	BtreeHeaderPage(BtreeHeaderPage &&other) = delete;
	BtreeHeaderPage &operator=(BtreeHeaderPage &&other) = delete;

	void SetRootPageId(const db::page_id_t &root_page_id) {
		root_page_id_ = root_page_id;
	}

	page_id_t GetRootPageId() const {
		return root_page_id_;
	}

	bool TreeIsEmpty() const {
		return root_page_id_ == INVALID_PAGE_ID;
	}

private:
	page_id_t root_page_id_ {INVALID_PAGE_ID};
};

} // namespace db
