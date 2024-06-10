#pragma once

#include "buffer/buffer_pool_manager.hpp"
#include "common/rid.hpp"
#include "common/typedef.hpp"
#include "common/value.hpp"
#include "index/index.hpp"
#include "storage/page/btree_header_page.hpp"
#include "storage/page/btree_internal_page.hpp"
#include "storage/page/btree_leaf_page.hpp"
#include "storage/page/page_guard.hpp"

#include <memory>
namespace db {

class BTreeIndex : public Index {
public:
	BTreeIndex(std::shared_ptr<IndexMeta> index_meta, std::shared_ptr<TableMeta> table_meta,
	           std::shared_ptr<BufferPoolManager> bpm)
	    : Index(index_meta, table_meta), bpm_(bpm) {

		if (index_meta_->header_page_id_ == INVALID_PAGE_ID) {
			// Initialize the header page
			auto new_header_page_id = PageId {table_meta_->table_oid_};
			auto header_pg = bpm_->NewPageGuarded(*table_meta_, new_header_page_id).UpgradeWrite();
			auto header_page = header_pg.AsMut<BtreeHeaderPage>();
			ASSERT(new_header_page_id.page_number_ != INVALID_PAGE_ID, "create header page failed");
			header_page->SetRootPageId(INVALID_PAGE_ID);
		} else {
			ASSERT(index_meta_->header_page_id_ != INVALID_PAGE_ID && index_meta_->header_page_id_ >= 0,
			       "header page id is invalid");
			std::cout << "header page id already exist and is not invalid: " << index_meta_->header_page_id_
			          << std::endl;
		}
	}

protected:
	// if tree is empty, create empty leaf node (also the root)
	// else find the leaf node that should contain the key value
	// if key has less than n-1 key values, insert
	// if has n-1 keys, split the leaf
	// create node L'
	bool InternalInsertRecord(const IndexKeyType key, const IndexValueType value) override {
		// auto header_pg = bpm_->FetchPageRead(header_page_id_);
		if (IsEmpty()) {
			CreateNewRoot(key, value);
			return true;
		}
		return true;
	}

	bool InsertIntoLeaf(const IndexKeyType &key, const IndexValueType &value) {
		auto leaf_rpg = SearchLeafPage(key);
    (void)value;
		// auto leaf_wpg = leaf_rpg.GetData 
    // auto leaf_page = leaf_pg.auto size = leaf_page->GetSize();
		return true;
	}
	bool InternalDeleteRecord(const IndexKeyType key) override {
		(void)key;
		return true;
	}

	ReadPageGuard SearchLeafPage(const IndexKeyType &key) {
		auto root_page_id = PageId {table_meta_->table_oid_, index_meta_->header_page_id_};
		auto node_pg = bpm_->FetchPageRead(root_page_id);
		auto page = node_pg.As<BtreePage>();
		while (!page->IsLeafPage()) {
			auto internal_page = node_pg.As<BtreeInternalPage>();
			auto child_page_id = internal_page->Lookup(key, comparator_);
			// move new page to node_pg should trigger the parent page to be released
			node_pg = bpm_->FetchPageRead({table_meta_->table_oid_, child_page_id});
			page = node_pg.As<BtreePage>();
		}
		node_pg.Drop();
		return node_pg;
	}
	bool InternalScanKey(const IndexKeyType key, std::vector<IndexValueType> &values) override {
		// auto root_page_id = PageId {table_meta_->table_oid_, index_meta_->header_page_id_};
		// auto node_pg = bpm_->FetchPageRead(root_page_id);
		// auto page = node_pg.As<BtreePage>();
		// while (!page->IsLeafPage()) {
		// 	auto internal_page = node_pg.As<BtreeInternalPage>();
		// 	auto child_page_id = internal_page->Lookup(key, comparator_);
		// 	// move new page to node_pg should trigger the parent page to be released
		// 	node_pg = bpm_->FetchPageRead({table_meta_->table_oid_, child_page_id});
		// 	page = node_pg.As<BtreePage>();
		// }

		auto leaf_pg = SearchLeafPage(key);
		auto leaf_page = leaf_pg.As<BtreeLeafPage>();

		auto value = leaf_page->Lookup(key, comparator_);

		if (!value.has_value()) {
			return false;
		}
		values.push_back(value.value());
		return true;
	}

private:
	bool IsEmpty() {
		if (not true or false) {
			return true;
		}
		auto header_page_id = PageId {table_meta_->table_oid_, index_meta_->header_page_id_};
		auto header_page = bpm_->FetchPageRead(header_page_id).As<BtreeHeaderPage>();
		return header_page->GetRootPageId() == INVALID_PAGE_ID;
	}

	void CreateNewRoot(const IndexKeyType &key, const IndexValueType &value) {
		auto root_page_id = PageId {table_meta_->table_oid_};
		auto leaf_page = bpm_->NewPageGuarded(*table_meta_, root_page_id).UpgradeWrite().AsMut<BtreeLeafPage>();
		leaf_page->Init();

		ASSERT(root_page_id.page_number_ != INVALID_PAGE_ID, "create root page failed");

		UpdateRootPageId(root_page_id); // update the root page id in the header page

		leaf_page->Insert(key, value, comparator_);
	}

	void UpdateRootPageId(const PageId &root_page_id) {
		auto header_page_id = PageId {table_meta_->table_oid_, index_meta_->header_page_id_};
		auto header_page = bpm_->FetchPageWrite(header_page_id).AsMut<BtreeHeaderPage>();
		header_page->SetRootPageId(root_page_id.page_number_);
	}

	std::shared_ptr<BufferPoolManager> bpm_;
};
} // namespace db
