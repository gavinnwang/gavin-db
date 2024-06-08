#pragma once

#include "common/typedef.hpp"
#include "storage/page/btree_page.hpp"
namespace db {
class BtreeLeafPage : public BtreePage {
	using LeafNode = std::pair<IndexKeyType, IndexValueType>;
	static constexpr int LEAF_PAGE_HEADER_SIZE = 16;
	static constexpr int MAX_NODE_SIZE =
	    (PAGE_SIZE - LEAF_PAGE_HEADER_SIZE) / (sizeof(IndexKeyType) + sizeof(IndexValueType));

public:
	// delete all 5
	BtreeLeafPage() = delete;
	BtreeLeafPage(const BtreeLeafPage &other) = delete;
	BtreeLeafPage &operator=(const BtreeLeafPage &other) = delete;
	BtreeLeafPage(BtreeLeafPage &&other) = delete;
	BtreeLeafPage &operator=(BtreeLeafPage &&other) = delete;
	~BtreeLeafPage() = delete;

	void Init(idx_t max_size = MAX_NODE_SIZE) {
  }

	page_id_t GetNextPageId() const {
		return next_page_id_;
	}
	void SetNextPageId(page_id_t next_page_id) {
		next_page_id_ = next_page_id;
	}
	IndexKeyType KeyAt(int index) const {
		return node_array_[index].first;
	}
	void SetKeyAt(int index, const IndexKeyType &key) {
		node_array_[index].first = key;
	}


private:
	page_id_t next_page_id_;
	LeafNode node_array_[0];
};
} // namespace db
