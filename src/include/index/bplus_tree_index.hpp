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
	BTreeIndex(IndexMeta &index_meta, TableMeta &table_meta, BufferPoolManager &bpm)
	    : Index(index_meta, table_meta), bpm_(bpm) {

		LOG_TRACE("BTreeIndex constructor called");

		if (index_meta_.header_page_id_ == INVALID_PAGE_ID) {
			LOG_TRACE("header page id is invalid, creating new header page");
			// Initialize the header page
			auto new_header_page_id = PageId {table_meta_.table_oid_};
			auto header_pg = bpm_.NewPageGuarded(*this, new_header_page_id).UpgradeWrite();
			auto &header_page = header_pg.AsMut<BtreeHeaderPage>();
			header_page.Init();
			assert(new_header_page_id.page_number_ >= INVALID_PAGE_ID);
			header_page.SetRootPageId(INVALID_PAGE_ID);
			index_meta_.header_page_id_ = new_header_page_id.page_number_;
		} else {
			LOG_DEBUG("header page id already exist and is not invalid: {}", index_meta_.header_page_id_);
		}
		assert(index_meta_.header_page_id_ >= 0);
		assert(comparator_ != nullptr);
	}

protected:
	bool InternalScanKey(const IndexKeyType key, std::vector<IndexValueType> &values) override {

		auto &header_raw_page = bpm_.FetchPage({table_meta_.table_oid_, index_meta_.header_page_id_});
		header_raw_page.RLatch();
		Transaction transaction {};

		auto &leaf_raw_page = SearchLeafPage(key, Operation::SEARCH, transaction, header_raw_page);

		LOG_TRACE("Traversed to leaf page found with page id: {}", leaf_raw_page.GetPageId().page_number_);

		const auto &leaf_page = leaf_raw_page.As<BtreeLeafPage>();

		// remove later
		LOG_TRACE("leaf: %s", leaf_page.ToString().c_str());

		auto value = leaf_page.Lookup(key, comparator_);

		leaf_raw_page.RUnlatch();
		bpm_.UnpinPage(leaf_raw_page.GetPageId(), false);

		if (!value.has_value()) {
			return false;
		}
		values.push_back(value.value());
		return true;
	}
	// if tree is empty, create empty leaf node (also the root)
	// else find the leaf node that should contain the key value
	// if key has less than n. key values, insert
	// if has n. keys, split the leaf
	// create node L'
	bool InternalInsertRecord(const IndexKeyType key, const IndexValueType value) override {
		// latch the root page
		auto &header_raw_page = bpm_.FetchPage({table_meta_.table_oid_, index_meta_.header_page_id_});
		header_raw_page.WLatch();
		Transaction transaction {};
		LOG_TRACE("Adding header page id {} into page set (header)", header_raw_page.GetPageId().page_number_);
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
		LOG_TRACE("Leaf node size: {} and max size {}", static_cast<int>(leaf_node.GetSize()),
		          static_cast<int>(leaf_node.GetMaxSize()));

		auto size = leaf_node.GetSize();
		leaf_node.Insert(key, value, comparator_);
		auto new_size = leaf_node.GetSize();

		// need to split and push to parent
		if (new_size >= leaf_node.GetMaxSize()) {
			LOG_TRACE("Need to split leaf and push to parent as size {} >= max size {}", static_cast<int>(new_size),
			          static_cast<int>(leaf_node.GetMaxSize()));

			// split insert success
			auto &sibling_leaf_node = Split(leaf_node);
			sibling_leaf_node.SetNextPageId(leaf_node.GetNextPageId());
			leaf_node.SetNextPageId(sibling_leaf_node.GetPageId());

			const auto &risen_key = sibling_leaf_node.KeyAt(0);
			LOG_TRACE("Insert leaf node {} into internal parent {} with risen key %s", leaf_node.GetPageId(),
			          leaf_node.GetParentPageId(), IndexKeyTypeToString(risen_key).c_str());
			InsertIntoParent(leaf_node, sibling_leaf_node, risen_key, transaction, header_page);

			leaf_page.WUnlatch();
			bpm_.UnpinPage(leaf_page.GetPageId(), true);
			bpm_.UnpinPage({table_meta_.table_oid_, sibling_leaf_node.GetPageId()}, true);

			return true;
		}
		LOG_TRACE("Don't need to split becase leaf size is {} and max size is {}", static_cast<int>(new_size),
		          static_cast<int>(leaf_node.GetMaxSize()));

		// don't need to split, release parent write latches one more time and do other clean up
		ReleaseParentWriteLatches(transaction);
		leaf_page.WUnlatch();
		bpm_.UnpinPage(leaf_page.GetPageId(), true);

		return new_size != size;
	}
	bool InternalDeleteRecord(const IndexKeyType key) override {
		auto &header_raw_page = bpm_.FetchPage({table_meta_.table_oid_, index_meta_.header_page_id_});
		header_raw_page.WLatch();
		Transaction transaction {};
		LOG_TRACE("Adding header page id {} into page set (header)", header_raw_page.GetPageId().page_number_);
		transaction.AddIntoPageSet(header_raw_page);

		auto &header_page = header_raw_page.AsMut<BtreeHeaderPage>();

		if (header_page.TreeIsEmpty()) {
			ReleaseParentWriteLatches(transaction);
			return false;
		}

		auto &leaf_page = SearchLeafPage(key, Operation::DELETE, transaction, header_raw_page);
		auto &leaf_node = leaf_page.AsMut<BtreeLeafPage>();

		(void)leaf_node;

		return true;
	}

	void InsertIntoParent(BtreePage &original_node, BtreePage &sibling_new_node, IndexKeyType key,
	                      Transaction &transaction, Page &header_page) {
		LOG_TRACE("Nodes {} and {} want to insert into their parent with key %s", original_node.GetPageId(),
		          sibling_new_node.GetPageId(), IndexKeyTypeToString(key).c_str());
		if (original_node.IsRootPage()) {
			LOG_TRACE("Split node is root, create new root");
			auto new_page_id = PageId {table_meta_.table_oid_};
			auto &new_internal_node = bpm_.NewPageGuarded(*this, new_page_id).UpgradeWrite().AsMut<BtreeInternalPage>();
			LOG_TRACE("Split node is root, create new root with page id {}", new_page_id.page_number_);
			new_internal_node.Init(new_page_id.page_number_, INVALID_PAGE_ID);
			new_internal_node.PopulateNewRoot(original_node.GetPageId(), key, sibling_new_node.GetPageId());

			LOG_TRACE("Setting parent page id of original node {} and new sibling node {} to new root {}",
			          original_node.GetPageId(), sibling_new_node.GetPageId(), new_page_id.page_number_);

			original_node.SetParentPageId(new_page_id.page_number_);
			sibling_new_node.SetParentPageId(new_page_id.page_number_);

			auto &header_node = header_page.AsMut<BtreeHeaderPage>();
			header_node.SetRootPageId(new_page_id.page_number_);

			ReleaseHeaderPageAndMarkDirty(transaction);
			ReleaseParentWriteLatches(transaction);
			return;
		}
		auto parent_page_id = original_node.GetParentPageId();
		LOG_TRACE("Split node is not root, insert key into internal parent with page id {}", parent_page_id);

		auto &parent_page = bpm_.FetchPage({table_meta_.table_oid_, parent_page_id});
		LOG_TRACE("Fetching parent page {}", parent_page_id);
		auto &parent_internal_node = parent_page.AsMut<BtreeInternalPage>();

		// parent has space
		if (parent_internal_node.GetSize() < parent_internal_node.GetMaxSize()) {
			LOG_TRACE("Internal parent {} has space, cur size: {}, max size: {}", parent_page_id,
			          static_cast<int>(parent_internal_node.GetSize()),
			          static_cast<int>(parent_internal_node.GetMaxSize()));

			parent_internal_node.InsertNodeAfter(original_node.GetPageId(), key, sibling_new_node.GetPageId());

			ReleaseParentWriteLatches(transaction);
			bpm_.UnpinPage(parent_page.GetPageId(), true);
			return;
		}
		// parent don't have space now have to split the parent internal node
		// currently the internal page size == internal max size which means that we have to allocate a new buffer space
		// inorder to prevent overflowing the current page
		LOG_TRACE("Internal parent {} is full, spliting parent", parent_page_id);
		auto buffer = std::vector<data_t>(INTERNAL_PAGE_HEADER_SIZE +
		                                  sizeof(InternalNode) * (parent_internal_node.GetSize() + 1));

		std::memcpy(buffer.data(), parent_page.GetData(),
		            INTERNAL_PAGE_HEADER_SIZE + sizeof(InternalNode) * (parent_internal_node.GetSize()));
		auto &copy_parent_node = reinterpret_cast<BtreeInternalPage &>(*buffer.data());
		copy_parent_node.InsertNodeAfter(original_node.GetPageId(), key, sibling_new_node.GetPageId());

		auto &parent_new_sibling_node = Split(copy_parent_node);
		LOG_TRACE("new sibling %s", parent_new_sibling_node.ToString().c_str());
		IndexKeyType new_key = parent_new_sibling_node.KeyAt(0);
		LOG_TRACE("new key %s", IndexKeyTypeToString(new_key).c_str());
		// by copying this (which include the header data), the size of the parent_internal_node will be updated
		std::memcpy(parent_page.GetData(), buffer.data(),
		            INTERNAL_PAGE_HEADER_SIZE + sizeof(InternalNode) * copy_parent_node.GetMinSize());
		// parent_internal_node.SetSize(copy_parent_node.GetMinSize());

		LOG_TRACE("new parent %s", parent_internal_node.ToString().c_str());
		LOG_TRACE("new sibling %s", parent_new_sibling_node.ToString().c_str());
		InsertIntoParent(parent_internal_node, parent_new_sibling_node, new_key, transaction, header_page);
		bpm_.UnpinPage(parent_page.GetPageId(), true);
		bpm_.UnpinPage({table_meta_.table_oid_, parent_new_sibling_node.GetPageId()}, true);
	}

	template <IsBtreeNode N>
	N &Split(N &node) {
		auto new_page_id = PageId {table_meta_.table_oid_};
		auto &new_page = bpm_.NewPage(*this, new_page_id);
		assert(new_page_id.page_number_ > 0);

		N &new_node = new_page.AsMut<N>();

		LOG_TRACE("Spliting a node with type %s", node.IsLeafPage() ? "Leaf" : "Internal");
		new_node.SetPageType(node.GetPageType());
		LOG_TRACE("Init the split sibling node with page id {} and parent id {}", new_page_id.page_number_,
		          node.GetParentPageId());
		new_node.Init(new_page_id.page_number_, node.GetParentPageId());

		if constexpr (IsLeafPage<N>::value) {
			LOG_TRACE("Splitting leaf node {}", node.GetPageId());
			node.MoveHalfTo(new_node);
		} else if constexpr (IsInternalPage<N>::value) {
			// not prepared for spliting internal node yet
			node.MoveHalfTo(new_node, bpm_, table_meta_.table_oid_);
		} else {
			UNREACHABLE("Invalid node type");
		}

		return new_node;
	}

	Page &SearchLeafPage(const IndexKeyType &key, Operation operation, Transaction &transaction, Page &header_page) {
		// auto root_page_id = PageId {table_meta_.table_oid_, GetRootPageId()};

		const auto &header_node = header_page.As<BtreeHeaderPage>();
		auto root_page_id = header_node.GetRootPageId();
		assert(root_page_id > 0);
		auto *page = &bpm_.FetchPage({table_meta_.table_oid_, root_page_id});
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

		LOG_TRACE("Search from root page id: {}", root_page_id);

		bool has_parent = false;
		while (!btree_node->IsLeafPage()) {
			const auto &internal_page = page->As<BtreeInternalPage>();
			has_parent = true;

			LOG_TRACE("Internal page id {} with size {}, parent id {}, max size {} content %s",
			          internal_page.GetPageId(),

			          static_cast<int>(internal_page.GetSize()), static_cast<int>(internal_page.GetParentPageId()),
			          static_cast<int>(internal_page.GetMaxSize()), internal_page.ToString().c_str());

			auto child_page_id = internal_page.Lookup(key, comparator_);
			assert(child_page_id > 0);
			LOG_TRACE("Search go to child: {}", child_page_id);
			// move new page to node_pg should trigger the parent page to be released
			auto *child_page = &bpm_.FetchPage({table_meta_.table_oid_, child_page_id});
			assert(child_page != nullptr);
			const auto *child_btree_node = &child_page->As<BtreePage>();

			// latch crabbing
			if (operation == Operation::SEARCH) {
				// unlatch parent and latch child
				child_page->RLatch();
				page->RUnlatch();
				bpm_.UnpinPage(page->GetPageId(), false);
			} else {
				child_page->WLatch();
				LOG_TRACE("Adding page id {} into page set", page->GetPageId().page_number_);
				transaction.AddIntoPageSet(*page);
				LOG_TRACE("Check if child node {} is safe to latch", child_page->GetPageId().page_number_);
				if (IsSafeNode(*child_btree_node, operation)) {
					LOG_TRACE("Child node {} is safe", child_page->GetPageId().page_number_);
					ReleaseParentWriteLatches(transaction);
				} else {
					LOG_TRACE("Child node {} is not safe", child_page->GetPageId().page_number_);
				}
			}

			page = child_page;
			btree_node = child_btree_node;
		}

		auto &leaf_node = (*page).AsMut<BtreeLeafPage>();
		if (has_parent) {
			assert(leaf_node.GetParentPageId() > 0);
		}
		LOG_TRACE("Leaf page id {}, size {}, parent id {}, max size {} content %s", leaf_node.GetPageId(),
		          static_cast<int>(leaf_node.GetSize()), static_cast<int>(leaf_node.GetParentPageId()),
		          static_cast<int>(leaf_node.GetMaxSize()), leaf_node.ToString().c_str());

		return *page;
	}

	[[nodiscard]] static bool IsSafeNode(const BtreePage &node, Operation operation) {
		assert(operation != Operation::SEARCH);
		// . because adding one more will not reach the threshold for splitting for leaf
		if (operation == Operation::INSERT) {
			if (node.IsLeafPage() && node.GetSize() < node.GetMaxSize()) {
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

	void ReleaseHeaderPageAndMarkDirty(Transaction &transaction) {
		auto page = transaction.GetPageSet()->front();
		assert(page.get().AsMut<BtreePage>().GetPageType() == IndexPageType::HEADER_PAGE);
		transaction.GetPageSet()->pop_front();
		page.get().WUnlatch();
		bpm_.UnpinPage(page.get().GetPageId(), true);
	}

	void ReleaseParentWriteLatches(Transaction &transaction) {
		LOG_TRACE("Releasing parent write latches");
		while (!transaction.GetPageSet()->empty()) {
			auto page = transaction.GetPageSet()->front();
			LOG_TRACE("Releasing page {} for table {}", page.get().GetPageId().page_number_,
			          page.get().GetPageId().table_id_);
			transaction.GetPageSet()->pop_front();
			page.get().WUnlatch();
			// dirty bit is false because we grab latch on child and
			// parent is unmodified and safe to release for this operation

			// when the page is the header this page can actually be modified when
			// the current root is split and a new root is created
			bpm_.UnpinPage(page.get().GetPageId(), false);
		}
	}

	void PrintTree() {
	}

private:
	// pass in the header page to satisfy the assumption that we have the write lock to the header page
	void CreateNewRoot(const IndexKeyType &key, const IndexValueType &value, BtreeHeaderPage &header_page) {
		auto root_page_id = PageId {table_meta_.table_oid_};
		auto &leaf_page = bpm_.NewPageGuarded(*this, root_page_id).UpgradeWrite().AsMut<BtreeLeafPage>();
		leaf_page.Init(root_page_id.page_number_, INVALID_PAGE_ID);
		assert(root_page_id.page_number_ > 0);
		LOG_TRACE("Root page id set to: {}", root_page_id.page_number_);
		header_page.SetRootPageId(root_page_id.page_number_);
		leaf_page.Insert(key, value, comparator_);
	}

	BufferPoolManager &bpm_;
};
} // namespace db
