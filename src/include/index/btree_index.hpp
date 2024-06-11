#pragma once

#include "buffer/buffer_pool_manager.hpp"
#include "common/rid.hpp"
#include "common/typedef.hpp"
#include "common/value.hpp"
#include "concurrency/transaction.hpp"
#include "index/index.hpp"
#include "storage/page/btree_header_page.hpp"
#include "storage/page/btree_internal_page.hpp"
#include "storage/page/btree_leaf_page.hpp"
#include "storage/page/page_guard.hpp"

#include <memory>
#include <utility>
namespace db {

class BTreeIndex : public Index {
	enum class Operation { SEARCH, INSERT, DELETE };

public:
	BTreeIndex(std::shared_ptr<IndexMeta> index_meta, std::shared_ptr<TableMeta> table_meta,
	           std::shared_ptr<BufferPoolManager> bpm)
	    : Index(std::move(index_meta), std::move(table_meta)), bpm_(std::move(bpm)) {

		if (index_meta_->header_page_id_ == INVALID_PAGE_ID) {
			// Initialize the header page
			auto new_header_page_id = PageId {table_meta_->table_oid_};
			auto header_pg = bpm_->NewPageGuarded(*table_meta_, new_header_page_id).UpgradeWrite();
			auto &header_page = header_pg.AsMut<BtreeHeaderPage>();
			ASSERT(new_header_page_id.page_number_ != INVALID_PAGE_ID, "create header page failed");
			header_page.SetRootPageId(INVALID_PAGE_ID);
			index_meta_->header_page_id_ = new_header_page_id.page_number_;
		} else {
			LOG_DEBUG("header page id already exist and is not invalid: %d", index_meta_->header_page_id_);
		}
		ASSERT(index_meta_->header_page_id_ != INVALID_PAGE_ID && index_meta_->header_page_id_ >= 0,
		       "header page id is invalid");
	}

protected:
	bool InternalScanKey(const IndexKeyType key, std::vector<IndexValueType> &values) override {

		auto &header_raw_page = bpm_->FetchPage({table_meta_->table_oid_, index_meta_->header_page_id_});
		header_raw_page.RLatch();
		Transaction transaction {};

		auto &leaf_raw_page = SearchLeafPage(key, Operation::SEARCH, transaction, header_raw_page);
		const auto &leaf_page = leaf_raw_page.As<BtreeLeafPage>();

		auto value = leaf_page.Lookup(key, comparator_);

		leaf_raw_page.RUnlatch();
		bpm_->UnpinPage(leaf_raw_page.GetPageId(), false);

		if (!value.has_value()) {
			return false;
		}
		values.push_back(value.value());
		return true;
	}
	// if tree is empty, create empty leaf node (also the root)
	// else find the leaf node that should contain the key value
	// if key has less than n-1 key values, insert
	// if has n-1 keys, split the leaf
	// create node L'
	bool InternalInsertRecord(const IndexKeyType key, const IndexValueType value) override {
		// latch the root page
		auto &header_raw_page = bpm_->FetchPage({table_meta_->table_oid_, index_meta_->header_page_id_});
		header_raw_page.WLatch();
		Transaction transaction {};
		transaction.AddIntoPageSet(header_raw_page);

		auto &header_page = header_raw_page.AsMut<BtreeHeaderPage>();

		if (header_page.TreeIsEmpty()) {
			LOG_TRACE("Inserting into empty tree, create new root.");
			CreateNewRoot(key, value, header_page);
			ReleaseParentWriteLatches(transaction);
			return true;
		}

		return InsertIntoLeaf(key, value, transaction, header_raw_page);
	}

	bool InsertIntoLeaf(const IndexKeyType &key, const IndexValueType &value, Transaction &transaction,
	                    Page &header_page) {
		auto &leaf_page = SearchLeafPage(key, Operation::INSERT, transaction, header_page);
		auto &leaf_node = leaf_page.AsMut<BtreeLeafPage>();

		auto size = leaf_node.GetSize();
		leaf_node.Insert(key, value, comparator_);
		auto new_size = leaf_node.GetSize();

		// need to split and push to parent
		if (new_size >= leaf_node.GetMaxSize()) {
			// split insert success
			// auto sibling_leaf_node = Split(node);
			return true;
		} // don't need to split, release parent write latches one more time and do other clean up
		ReleaseParentWriteLatches(transaction);
		leaf_page.WUnlatch();
		bpm_->UnpinPage(leaf_page.GetPageId(), false);

		return new_size != size;
	}
	bool InternalDeleteRecord(const IndexKeyType key) override {
		(void)key;
		return true;
	}

	Page &SearchLeafPage(const IndexKeyType &key, Operation operation, Transaction &transaction, Page &header_page) {
		// auto root_page_id = PageId {table_meta_->table_oid_, GetRootPageId()};

		const auto &header_node = header_page.As<BtreeHeaderPage>();
		auto root_page_id = header_node.GetRootPageId();
		assert(root_page_id > 0);
		auto *page = &bpm_->FetchPage({table_meta_->table_oid_, root_page_id});
		const auto *btree_node = &page->As<BtreePage>();
		assert(page != nullptr);
		// get latch on first node
		if (operation == Operation::SEARCH) {
			// unlatch the parent which is root
			header_page.RUnlatch();
			page->RLatch();
		} else {
			// for insert and delete
			page->WLatch();
		}

		while (!btree_node->IsLeafPage()) {
			const auto &internal_page = page->As<BtreeInternalPage>();
			auto child_page_id = internal_page.Lookup(key, comparator_);
			assert(child_page_id > 0);
			LOG_TRACE("Search go to child: %d", child_page_id);
			// move new page to node_pg should trigger the parent page to be released
			auto *child_page = &bpm_->FetchPage({table_meta_->table_oid_, child_page_id});
			assert(child_page != nullptr);
			const auto *child_btree_node = &child_page->As<BtreePage>();

			// latch crabbing
			if (operation == Operation::SEARCH) {
				// unlatch parent and latch child
				child_page->RLatch();
				page->RUnlatch();
				bpm_->UnpinPage(page->GetPageId(), false);
			} else {
				child_page->WLatch();
				transaction.AddIntoPageSet(*page);
				// if child node is safe to latch, release parent latches
				if (IsSafeNode(*child_btree_node, operation)) {
					ReleaseParentWriteLatches(transaction);
				}
			}

			page = child_page;
			btree_node = child_btree_node;
		}

		return *page;
	}

	[[nodiscard]] static bool IsSafeNode(const BtreePage &node, Operation operation) {
		assert(operation != Operation::SEARCH);
		// -1 because adding one more will not reach the threshold for splitting for leaf
		if (operation == Operation::INSERT) {
			if (node.IsLeafPage() && node.GetSize() < node.GetMaxSize() - 1) {
				return true;
			}
			// if internal node have room for one more key value, then it is safe
			if (!node.IsLeafPage() && node.GetSize() < node.GetMaxSize()) {
				return true;
			}
		} else {
			if (node.GetSize() > node.GetMinSize()) {
				return true;
			}
		}
		return false;
	}

	void ReleaseParentWriteLatches(Transaction &transaction) {
		while (!transaction.GetPageSet()->empty()) {
			auto page = transaction.GetPageSet()->front();
			transaction.GetPageSet()->pop_front();
			page.get().WUnlatch();
			bpm_->UnpinPage(page.get().GetPageId(), false);
		}
	}

private:
	// bool IsEmpty() {
	// 	auto header_page_id = PageId {table_meta_->table_oid_, index_meta_->header_page_id_};
	// 	ASSERT(header_page_id.page_number_ != INVALID_PAGE_ID, "header page id is invalid");
	// 	auto &header_page = bpm_->FetchPageRead(header_page_id).As<BtreeHeaderPage>();
	// 	return header_page.GetRootPageId() == INVALID_PAGE_ID;
	// }

	// pass in the header page to satisfy the assumption that we have the write lock to the header page
	void CreateNewRoot(const IndexKeyType &key, const IndexValueType &value, BtreeHeaderPage &header_page) {
		auto root_page_id = PageId {table_meta_->table_oid_};
		auto &leaf_page = bpm_->NewPageGuarded(*table_meta_, root_page_id).UpgradeWrite().AsMut<BtreeLeafPage>();
		leaf_page.Init();

		ASSERT(root_page_id.page_number_ != INVALID_PAGE_ID, "create root page failed");

		UpdateRootPageId(root_page_id, header_page); // update the root page id in the header page

		leaf_page.Insert(key, value, comparator_);
	}

	// pass in the header page to satisfy the assumption that we have the write lock to the header page
	void UpdateRootPageId(const PageId &root_page_id, BtreeHeaderPage &header_page) {
		assert(index_meta_->header_page_id_ >= 0);
		auto header_page_id = PageId {table_meta_->table_oid_, index_meta_->header_page_id_};
		// auto &header_page = bpm_->FetchPageWrite(header_page_id).AsMut<BtreeHeaderPage>();
		header_page.SetRootPageId(root_page_id.page_number_);
	}

	// page_id_t GetRootPageId() {
	// 	assert(index_meta_->header_page_id_ >= 0);
	// 	auto header_page_id = PageId {table_meta_->table_oid_, index_meta_->header_page_id_};
	// 	auto &header_page = bpm_->FetchPageRead(header_page_id).As<BtreeHeaderPage>();
	// 	return header_page.GetRootPageId();
	// }

	std::shared_ptr<BufferPoolManager> bpm_;
};
} // namespace db
