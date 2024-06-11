#pragma once

#include "common/exception.hpp"
#include "common/macros.hpp"
#include "common/typedef.hpp"
#include "index/index.hpp"
#include "storage/page/btree_page.hpp"

#include <optional>
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
		SetSize(0);
		SetMaxSize(max_size);
		SetNextPageId(INVALID_PAGE_ID);
		SetPageType(IndexPageType::LEAF_PAGE);
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
	void Insert(IndexKeyType key, IndexValueType value, const Comparator &comparator) {
		if (GetSize() == GetMaxSize()) {
			throw RuntimeException("Leaf node is full, shouldve split bruh");
		}
		LOG_TRACE("Inserting into leaf node");
		auto key_idx = FindKeyIndex(key, comparator);
		for (idx_t i = key_idx; i < GetSize(); i++) {
			node_array_[i + 1] = node_array_[i];
		}

		node_array_[key_idx] = std::make_pair(key, value);
	}
	idx_t FindKeyIndex(IndexKeyType key, const Comparator &comparator) const {
		idx_t left = 0;
		idx_t right = GetSize();

		while (left < right) {
			idx_t mid = left + (right - left) / 2;
			if (comparator(node_array_[mid].first, key)) {
				left = mid + 1;
			} else {
				right = mid;
			}
		}

		return left;
	}
	std::optional<IndexValueType> Lookup(const IndexKeyType &key, const Comparator &comparator) const {
		idx_t target_index = FindKeyIndex(key, comparator);
		if (target_index < GetSize() && node_array_[target_index].first == key) {
			return node_array_[target_index].second;
		}
		return std::nullopt;
	}

private:
	page_id_t next_page_id_ {INVALID_PAGE_ID};
	LeafNode node_array_[0];
};
} // namespace db
