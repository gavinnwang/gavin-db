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

private:
	db::page_id_t root_page_id_;
};

} // namespace db
