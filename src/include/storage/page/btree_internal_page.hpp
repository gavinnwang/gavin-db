#pragma once

#include "common/config.hpp"
#include "common/macros.hpp"
#include "common/rid.hpp"
#include "common/typedef.hpp"
#include "common/value.hpp"
#include "index/index.hpp"
#include "storage/page/btree_page.hpp"

#include <array>

namespace db {
class BtreeInternalPage : public BtreePage {

	using InternalNode = std::pair<IndexKeyType, IndexValueType>;

	static constexpr int INTERNAL_PAGE_HEADER_SIZE = 12;
	static constexpr int MAX_NODE_SIZE =
	    (PAGE_SIZE - INTERNAL_PAGE_HEADER_SIZE) / (sizeof(IndexKeyType) + sizeof(IndexValueType));

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
	}

	IndexKeyType KeyAt(int index) const {
		return node_array_[index].first;
	}

	void SetKeyAt(int index, const IndexKeyType &key) {
		node_array_[index].first = key;
	}

	idx_t ValueIndex(const IndexValueType &value) const {
		for (idx_t i = 0; i < GetSize(); i++) {
			if (node_array_[i].second == value) {
				return i;
			}
		}
		UNREACHABLE("Value not found");
	}

	IndexValueType ValueAt(int index) const {
		return node_array_[index].second;
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
			result += std::to_string(ValueAt(i).data()[0]);
		}
		result += ")";
		return result;
	}

private:
	static_assert(std::is_trivially_copyable_v<IndexKeyType> == true);
	static_assert(std::is_trivially_copyable_v<IndexValueType> == true);

	InternalNode node_array_[0];
};
} // namespace db
