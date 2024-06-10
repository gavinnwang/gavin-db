#pragma once

#include "common/config.hpp"
#include "common/macros.hpp"
#include "common/rid.hpp"
#include "common/typedef.hpp"
#include "common/value.hpp"
#include "index/index.hpp"
#include "storage/page/btree_page.hpp"

#include <algorithm>
#include <array>

namespace db {
class BtreeInternalPage : public BtreePage {

	using InternalNode = std::pair<IndexKeyType, InternalValueType>;

	static constexpr int INTERNAL_PAGE_HEADER_SIZE = 12;
	static constexpr int MAX_NODE_SIZE =
	    (PAGE_SIZE - INTERNAL_PAGE_HEADER_SIZE) / (sizeof(IndexKeyType) + sizeof(InternalValueType));

public:
	BtreeInternalPage() = delete;
	BtreeInternalPage(const BtreeInternalPage &other) = delete;
	BtreeInternalPage &operator=(const BtreeInternalPage &other) = delete;
	BtreeInternalPage(BtreeInternalPage &&other) = delete;
	BtreeInternalPage &operator=(BtreeInternalPage &&other) = delete;
	~BtreeInternalPage() = delete;

	void Init(int max_size = MAX_NODE_SIZE) {
		SetSize(0);
		SetMaxSize(max_size);
		SetPageType(IndexPageType::INTERNAL_PAGE);
	}

	IndexKeyType KeyAt(int index) const {
		return node_array_[index].first;
	}

	void SetKeyAt(int index, const IndexKeyType &key) {
		node_array_[index].first = key;
	}

	idx_t ValueIndex(const InternalValueType &value) const {
		for (idx_t i = 0; i < GetSize(); i++) {
			if (node_array_[i].second == value) {
				return i;
			}
		}
		UNREACHABLE("Value not found");
	}

	InternalValueType ValueAt(int index) const {
		return node_array_[index].second;
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

private:
	static_assert(std::is_trivially_copyable_v<IndexKeyType> == true);
	static_assert(std::is_trivially_copyable_v<InternalValueType> == true);

	InternalNode node_array_[0];
};
} // namespace db
