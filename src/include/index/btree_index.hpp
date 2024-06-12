#pragma once

#include "buffer/buffer_pool_manager.hpp"
#include "common/config.hpp"
#include "common/logger.hpp"
#include "common/macros.hpp"
#include "common/typedef.hpp"
#include "concurrency/transaction.hpp"
#include "index/index.hpp"
#include "storage/page/btree_header_page.hpp"
#include "storage/page/btree_internal_page.hpp"
#include "storage/page/btree_leaf_page.hpp"
#include "storage/page/page_guard.hpp"

#include <memory>
#include <utility>
namespace db {

// Define the trait to identify leaf and internal pages
template <typename T>
struct IsLeafPage : std::false_type {};

template <>
struct IsLeafPage<BtreeLeafPage> : std::true_type {};

template <typename T>
struct IsInternalPage : std::false_type {};

template <>
struct IsInternalPage<BtreeInternalPage> : std::true_type {};
template <typename T>
concept IsBtreeNode = std::is_same_v<T, BtreeLeafPage> || std::is_same_v<T, BtreeInternalPage>;

class BTreeIndex : public Index {
	enum class Operation { SEARCH, INSERT, DELETE };

public:
	BTreeIndex(std::shared_ptr<IndexMeta> index_meta, std::shared_ptr<TableMeta> table_meta,
	           const std::shared_ptr<BufferPoolManager> &bpm)
	    : Index(std::move(index_meta), std::move(table_meta)), bpm_(bpm) {

		LOG_TRACE("BTreeIndex constructor called");

		if (index_meta_->header_page_id_ == INVALID_PAGE_ID) {
			LOG_TRACE("header page id is invalid, creating new header page");
			// Initialize the header page
			auto new_header_page_id = PageId {table_meta_->table_oid_};
			auto header_pg = bpm_->NewPageGuarded(*index_meta_, new_header_page_id).UpgradeWrite();
			auto &header_page = header_pg.AsMut<BtreeHeaderPage>();
			assert(new_header_page_id.page_number_ >= INVALID_PAGE_ID);
			header_page.SetRootPageId(INVALID_PAGE_ID);
			index_meta_->header_page_id_ = new_header_page_id.page_number_;
		} else {
			LOG_DEBUG("header page id already exist and is not invalid: %d", index_meta_->header_page_id_);
		}
		assert(index_meta_->header_page_id_ >= 0);
		assert(comparator_ != nullptr);
	}

protected:
	bool InternalScanKey(const IndexKeyType key, std::vector<IndexValueType> &values) override {

		auto &header_raw_page = bpm_->FetchPage({table_meta_->table_oid_, index_meta_->header_page_id_});
		header_raw_page.RLatch();
		Transaction transaction {};

		auto &leaf_raw_page = SearchLeafPage(key, Operation::SEARCH, transaction, header_raw_page);

		LOG_TRACE("Traversed to leaf page found with page id: %d", leaf_raw_page.GetPageId().page_number_);

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
		LOG_TRACE("Adding header page id %d into page set (header)", header_raw_page.GetPageId().page_number_);
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
		LOG_TRACE("Leaf node size: %d and max size %d", static_cast<int>(leaf_node.GetSize()),
		          static_cast<int>(leaf_node.GetMaxSize()));

		auto size = leaf_node.GetSize();
		leaf_node.Insert(key, value, comparator_);
		auto new_size = leaf_node.GetSize();

		// need to split and push to parent
		if (new_size >= leaf_node.GetMaxSize()) {
			LOG_TRACE("Need to split leaf and push to parent as size %d >= max size %d", static_cast<int>(new_size),
			          static_cast<int>(leaf_node.GetMaxSize()));

			// split insert success
			auto &sibling_leaf_node = Split(leaf_node);
			sibling_leaf_node.SetNextPageId(leaf_node.GetNextPageId());
			leaf_node.SetNextPageId(sibling_leaf_node.GetPageId());

			const auto &risen_key = sibling_leaf_node.KeyAt(0);
			LOG_TRACE("Insert leaf node %d into internal parent %d with risen key %s", leaf_node.GetPageId(),
			          leaf_node.GetParentPageId(), IndexKeyTypeToString(risen_key).c_str());
			InsertIntoParent(leaf_node, sibling_leaf_node, risen_key, transaction, header_page);

			leaf_page.WUnlatch();
			bpm_->UnpinPage(leaf_page.GetPageId(), true);
			bpm_->UnpinPage({table_meta_->table_oid_, sibling_leaf_node.GetPageId()}, true);

			return true;
		}

		// don't need to split, release parent write latches one more time and do other clean up
		ReleaseParentWriteLatches(transaction);
		leaf_page.WUnlatch();
		bpm_->UnpinPage(leaf_page.GetPageId(), false);

		return new_size != size;
	}
	bool InternalDeleteRecord(const IndexKeyType key) override {
		(void)key;
		return true;
	}

	void InsertIntoParent(BtreePage &original_node, BtreePage &leaf_sibling_new_node, IndexKeyType key,
	                      Transaction &transaction, Page &header_page) {
		if (original_node.IsRootPage()) {
			auto new_page_id = PageId {table_meta_->table_oid_};
			auto &new_internal_node =
			    bpm_->NewPageGuarded(*index_meta_, new_page_id).UpgradeWrite().AsMut<BtreeInternalPage>();
			LOG_TRACE("Split node is root, create new root with page id %d", new_page_id.page_number_);
			new_internal_node.Init(new_page_id.page_number_, INVALID_PAGE_ID);
			new_internal_node.PopulateNewRoot(original_node.GetPageId(), key, leaf_sibling_new_node.GetPageId());

			LOG_TRACE("Setting parent page id of original node %d and new sibling node %d to new root %d",
			          original_node.GetPageId(), leaf_sibling_new_node.GetPageId(), new_page_id.page_number_);

			original_node.SetParentPageId(new_page_id.page_number_);
			leaf_sibling_new_node.SetParentPageId(new_page_id.page_number_);

			auto &header_node = header_page.AsMut<BtreeHeaderPage>();
			header_node.SetRootPageId(new_page_id.page_number_);

			ReleaseParentWriteLatches(transaction);
			return;
		}
		auto parent_page_id = original_node.GetParentPageId();
		LOG_TRACE("Split node is not root, insert key into internal parent with page id %d", parent_page_id);

		auto &parent_page = bpm_->FetchPage({table_meta_->table_oid_, parent_page_id});
		auto &parent_internal_node = parent_page.AsMut<BtreeInternalPage>();

		// parent has space
		if (parent_internal_node.GetSize() < parent_internal_node.GetMaxSize()) {
			LOG_TRACE("Internal parent %d has space, cur size: %d, max size: %d", parent_page_id,
			          static_cast<int>(parent_internal_node.GetSize()),
			          static_cast<int>(parent_internal_node.GetMaxSize()));

			// parent_internal_node.InsertNodeAfter(original_node.GetPageId(), key, leaf_sibling_new_node.GetPageId());
			ReleaseParentWriteLatches(transaction);
			return;
		}
		// parent don't have space now have to split the parent internal node
		assert(false);

		// currently the internal page size == internal max size which means that we have to allocate a new buffer space
		// inorder to prevent overflowing the current page
	}

	template <IsBtreeNode N>
	N &Split(N &node) {
		auto new_page_id = PageId {table_meta_->table_oid_};
		auto new_page = bpm_->NewPageGuarded(*index_meta_, new_page_id).UpgradeWrite();
		assert(new_page_id.page_number_ > 0);
		N &new_node = new_page.AsMut<N>();

		LOG_TRACE("Spliting a node with type %s", node.IsLeafPage() ? "Leaf" : "Internal");
		new_node.SetPageType(node.GetPageType());
		LOG_TRACE("Init the split sibling node with page id %d and parent id %d", new_page_id.page_number_,
		          node.GetParentPageId());
		new_node.Init(new_page_id.page_number_, node.GetParentPageId());

		if constexpr (IsLeafPage<N>::value) {
			node.MoveHalfTo(new_node);
		} else if constexpr (IsInternalPage<N>::value) {
			node.MoveHalfTo(new_node, bpm_, table_meta_->table_oid_);
		} else {
			UNREACHABLE("Invalid node type");
		}

		return new_node;
	}

	Page &SearchLeafPage(const IndexKeyType &key, Operation operation, Transaction &transaction, Page &header_page) {
		// auto root_page_id = PageId {table_meta_->table_oid_, GetRootPageId()};

		const auto &header_node = header_page.As<BtreeHeaderPage>();
		auto root_page_id = header_node.GetRootPageId();
		LOG_TRACE("Search from root page id: %d", root_page_id);
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
				// add parent to page set
				LOG_TRACE("Adding page id %d into page set", page->GetPageId().page_number_);
				transaction.AddIntoPageSet(*page);
				// if child node is safe to latch, release parent latches
				LOG_TRACE("Check if child node %d is safe to latch", child_page->GetPageId().page_number_);
				if (IsSafeNode(*child_btree_node, operation)) {
					LOG_TRACE("Child node %d is safe", child_page->GetPageId().page_number_);
					ReleaseParentWriteLatches(transaction);
				} else {
					LOG_TRACE("Child node %d is not safe", child_page->GetPageId().page_number_);
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
			// for delete operation
			if (node.GetSize() > node.GetMinSize()) {
				return true;
			}
		}
		return false;
	}

	void ReleaseParentWriteLatches(Transaction &transaction) {
		LOG_TRACE("Releasing parent write latches");
		while (!transaction.GetPageSet()->empty()) {
			auto page = transaction.GetPageSet()->front();
			LOG_TRACE("Releasing page %d for table %d", page.get().GetPageId().page_number_,
			          page.get().GetPageId().table_id_);
			transaction.GetPageSet()->pop_front();
			page.get().WUnlatch();
			// dirty bit is false because we grab latch on child and
			// parent is unmodified and safe to release for this operation
			bpm_->UnpinPage(page.get().GetPageId(), false);
		}
	}

private:
	// pass in the header page to satisfy the assumption that we have the write lock to the header page
	void CreateNewRoot(const IndexKeyType &key, const IndexValueType &value, BtreeHeaderPage &header_page) {
		auto root_page_id = PageId {table_meta_->table_oid_};
		auto &leaf_page = bpm_->NewPageGuarded(*index_meta_, root_page_id).UpgradeWrite().AsMut<BtreeLeafPage>();
		leaf_page.Init(root_page_id.page_number_, INVALID_PAGE_ID);
		assert(root_page_id.page_number_ > 0);
		LOG_TRACE("Root page id set to: %d", root_page_id.page_number_);
		header_page.SetRootPageId(root_page_id.page_number_);
		leaf_page.Insert(key, value, comparator_);
	}

	const std::shared_ptr<BufferPoolManager> &bpm_;
};
} // namespace db
