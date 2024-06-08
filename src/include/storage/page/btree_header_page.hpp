#pragma once

#include "common/typedef.hpp"
namespace bustub {

class BtreeHeaderPage {
public:
	BtreeHeaderPage() = delete;
	BtreeHeaderPage(const BtreeHeaderPage &other) = delete;
	BtreeHeaderPage &operator=(const BtreeHeaderPage &other) = delete;
	~BtreeHeaderPage() = delete;
	BtreeHeaderPage(BtreeHeaderPage &&other) = delete;
	BtreeHeaderPage &operator=(BtreeHeaderPage &&other) = delete;

	db::page_id_t root_page_id_;
};

} // namespace bustub
