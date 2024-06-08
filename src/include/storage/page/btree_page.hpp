#pragma once

#include "common/typedef.hpp"
namespace db {

enum class IndexPageType { INVALID_INDEX_PAGE = 0, LEAF_PAGE, INTERNAL_PAGE };

class BtreePage {
public:
	BtreePage() = delete;
	BtreePage(const BtreePage &other) = delete;
	BtreePage &operator=(const BtreePage &other) = delete;
	BtreePage(BtreePage &&other) = delete;
	BtreePage &operator=(BtreePage &&other) = delete;
	~BtreePage() = delete;

	idx_t GetSize() const {
		return size_;
	}
	void SetSize(int size) {
		size_ = size;
	}
	void IncreaseSize(int amount) {
		size_ += amount;
	}

	int GetMaxSize() const {
		return max_size_;
	}
	void SetMaxSize(int max_size) {
		max_size_ = max_size;
	}
	int GetMinSize() const {
		return max_size_ / 2;
	}

private:
	IndexPageType page_type_;
	int size_;
	int max_size_;
};

} // namespace db
