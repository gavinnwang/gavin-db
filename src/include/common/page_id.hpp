#pragma once

#include "common/config.hpp"
#include "common/typedef.hpp"

#include <functional>
namespace db {

enum class PageType : uint8_t { INVALID_PAGE = 0, DATA_PAGE = 1, HASH_INDEX_PAGE = 2, BTREE_INDEX_PAGE = 3 };

struct PageId {
	page_id_t page_number_;
	table_oid_t table_id_;
	PageType page_type_;

	explicit PageId() : page_number_(INVALID_PAGE_ID), table_id_(INVALID_TABLE_OID) {
	}
	explicit PageId(table_oid_t table_id) : page_number_(INVALID_PAGE_ID), table_id_(table_id) {};
	PageId(table_oid_t table_id, page_id_t page_number) : page_number_(page_number), table_id_(table_id) {};
	// Define equality operator for PageID
	bool operator==(const PageId &other) const {
		return table_id_ == other.table_id_ && page_number_ == other.page_number_;
	}
};

struct PageId_Hash {
	std::size_t operator()(const PageId &page_id) const {
		return std::hash<int>()(page_id.table_id_) ^ std::hash<int>()(page_id.page_number_);
	}
};

} // namespace db
