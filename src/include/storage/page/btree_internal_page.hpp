#pragma once

#include "buffer/buffer_pool_manager.hpp"
#include "common/config.hpp"
#include "common/logger.hpp"
#include "common/typedef.hpp"
#include "index/index.hpp"
#include "storage/page/btree_page.hpp"

#include <algorithm>
#include <array>

namespace db {
static constexpr int INTERNAL_PAGE_HEADER_SIZE = 32;
static constexpr int INTERNAL_MAX_NODE_SIZE =
    (PAGE_SIZE - INTERNAL_PAGE_HEADER_SIZE) / (sizeof(IndexKeyType) + sizeof(InternalValueType));
class BtreeInternalPage : public BtreePage {

	using InternalNode = std::pair<IndexKeyType, InternalValueType>;

public:
	BtreeInternalPage() = delete;
	BtreeInternalPage(const BtreeInternalPage &other) = delete;
	BtreeInternalPage &operator=(const BtreeInternalPage &other) = delete;
	BtreeInternalPage(BtreeInternalPage &&other) = delete;
	BtreeInternalPage &operator=(BtreeInternalPage &&other) = delete;
	~BtreeInternalPage() = delete;

	void Init(page_id_t page_id, page_id_t parent_id) {
		SetPageType(IndexPageType::INTERNAL_PAGE);
		SetPageId(page_id);
		SetSize(0);
		SetMaxSize(INTERNAL_MAX_NODE_SIZE);
		SetParentPageId(parent_id);
		LOG_TRACE("Setting internal page size to 0 and max size to %d", static_cast<int>(INTERNAL_MAX_NODE_SIZE));
	}

	IndexKeyType KeyAt(int index) const {
		return node_array_[index].first;
	}

	void SetKeyAt(int index, const IndexKeyType &key) {
		node_array_[index].first = key;
	}

	idx_t ValueIndex(const InternalValueType &value) const {
		auto it = std::find_if(node_array_, node_array_ + GetSize(),
		                       [&value](const auto &pair) { return pair.second == value; });
		return std::distance(node_array_, it);
	}

	InternalValueType ValueAt(int index) const {
		return node_array_[index].second;
	}

	void SetValueAt(int index, const InternalValueType &value) {
		node_array_[index].second = value;
	}

	InternalValueType Lookup(const IndexKeyType &key, const Comparator &comparator) const {
		// ignore the first key
		// lwoer bound returns first it where comp(*it, key) is false
		auto target =
		    std::lower_bound(node_array_ + 1, node_array_ + GetSize(), key,
		                     [&comparator](const auto &pair, auto key) { return comparator(pair.first, key) < 0; });
		// if target is last then lookup key is larger than all keys
		if (target == node_array_ + GetSize()) {
			return ValueAt(GetSize() - 1);
			// if target is the same as the search key go to that value due our internal
			// node convention
		} else if (comparator(key, target->first) == 0) {
			return target->second;
		} else {
			return std::prev(target)->second;
		}
	}

	void PopulateNewRoot(const InternalValueType &old_value, const IndexKeyType &new_key,
	                     const InternalValueType &new_value) {
		SetValueAt(0, old_value);
		SetKeyAt(1, new_key);
		SetValueAt(1, new_value);
		SetSize(2);
	}

	std::string ToString() const {
		std::string result = "(";
		bool first = true;
		for (idx_t i = 0; i < GetSize(); i++) {
			if (!first) {
				result += ", ";
			}
			first = false;
			result += std::to_string(KeyAt(i).data()[0]);

			result += ":";
			result += std::to_string(ValueAt(i));
		}
		result += ")";
		return result;
	}

	idx_t InsertNodeAfter(const InternalValueType &old_value, const IndexKeyType &new_key,
	                      const InternalValueType &new_value) {
		LOG_TRACE("Inserting key %s after value %d", IndexKeyTypeToString(new_key).c_str(), new_value);

		auto new_value_idx = ValueIndex(old_value) + 1;
		std::move_backward(node_array_ + new_value_idx, node_array_ + GetSize(), node_array_ + GetSize() + 1);

		node_array_[new_value_idx].first = new_key;
		node_array_[new_value_idx].second = new_value;
		IncreaseSize(1);

		return GetSize();
	}

	void MoveHalfTo(BtreeInternalPage &recipient, const std::shared_ptr<BufferPoolManager> &bpm,
	                table_oid_t table_oid) {
		idx_t start_split_indx = GetMinSize();
		idx_t original_size = GetSize();
		SetSize(start_split_indx);
		recipient.CopyNFrom(node_array_ + start_split_indx, original_size - start_split_indx, bpm, table_oid);
	}

	void CopyNFrom(InternalNode *items, idx_t size, const std::shared_ptr<BufferPoolManager> &bpm,
	               table_oid_t table_oid) {
		std::copy(items, items + size, node_array_ + GetSize());

		// because the recipient got the child originally referred by the owner the recipient have to be set the parent
		// of those child leaf nodes
		for (idx_t i = 0; i < size; i++) {
			auto &node_wpg = bpm->FetchPageWrite({table_oid, ValueAt(i + GetSize())}).AsMut<BtreePage>();
			node_wpg.SetParentPageId(GetParentPageId());
		}
	}

private:
	static_assert(std::is_trivially_copyable_v<IndexKeyType>);
	static_assert(std::is_trivially_copyable_v<InternalValueType>);

	InternalNode node_array_[0];
};
static_assert(sizeof(BtreeInternalPage) == INTERNAL_PAGE_HEADER_SIZE);
} // namespace db
