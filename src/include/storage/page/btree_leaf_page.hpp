#pragma once

#include "common/exception.hpp"
#include "common/logger.hpp"
#include "common/typedef.hpp"
#include "index/btree_index.hpp"
#include "index/index.hpp"
#include "storage/page/btree_page.hpp"

#include <algorithm>
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
		LOG_TRACE("Setting size to 0 and max size to %d", static_cast<int>(max_size));
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
	void Insert(const IndexKeyType &key, const IndexValueType &value, const Comparator &comparator) {
		if (GetSize() == GetMaxSize()) {
			throw RuntimeException("Leaf node is full, shouldve split bruh");
		}
		assert(key[0]);
		auto key_idx = FindKeyIndex(key, comparator);

		if (comparator(node_array_[key_idx].first, key) == 0) {
			LOG_TRACE("Key %s already exists, updating value to %s", IndexKeyTypeToString(key).c_str(),
			          value.ToString().c_str());
			// todo(gavinnwang): update the value of the key?
			return;
		}
		LOG_TRACE("Inserting key %s at index %d with val: %s", IndexKeyTypeToString(key).c_str(),
		          static_cast<int>(key_idx), value.ToString().c_str());

		if (key_idx == GetSize()) {
			*(node_array_ + key_idx) = {key, value};
		} else {
			// shift everything at and after the key idx back one to make space
			std::move_backward(node_array_ + key_idx, node_array_ + GetSize(), node_array_ + GetSize() + 1);
			*(node_array_ + key_idx) = {key, value};
		}
		IncreaseSize(1);
	}
	idx_t FindKeyIndex(const IndexKeyType &key, const Comparator &comparator) const {
		auto target_idx =
		    std::lower_bound(node_array_, node_array_ + GetSize(), key,
		                     [&comparator](const auto &pair, auto k) { return comparator(pair.first, k) < 0; });
		return std::distance(node_array_, target_idx);
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
