#pragma once

#include "buffer/buffer_pool_manager.hpp"
#include "common/rid.hpp"
#include "common/typedef.hpp"
#include "common/value.hpp"
#include "index/index.hpp"
#include "storage/page/btree_header_page.hpp"
#include "storage/page/btree_leaf_page.hpp"

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
			header_page->SetRootPageId(new_header_page_id.page_number_);
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
	bool InternalInsertRecord(const IndexKeyType key, const RID rid) override {
		// auto header_pg = bpm_->FetchPageRead(header_page_id_);

		return true;
	}
	bool InternalDeleteRecord(const IndexKeyType key) override {
		return true;
	}
	bool InternalScanKey(const IndexKeyType key, std::vector<RID> &rids) override {
		return true;
	}

private:
	void UpdateRootPageId(const PageId &root_page_id) {
		auto header_page_id = PageId {table_meta_->table_oid_, index_meta_->header_page_id_};
		auto header_pg = bpm_->FetchPageWrite(header_page_id);
		auto header_page = header_pg.AsMut<BtreeHeaderPage>();

		header_page->SetRootPageId(root_page_id.page_number_);
	}

	void CreateNewRoot(const IndexKeyType &key, const IndexValueType &value) {
		auto root_page_id = PageId {table_meta_->table_oid_};
		auto leaf_wpg = bpm_->NewPageGuarded(*table_meta_, root_page_id).UpgradeWrite();
		auto leaf_page = leaf_wpg.AsMut<BtreeLeafPage>();
		leaf_page->Init();

		ASSERT(root_page_id.page_number_ != INVALID_PAGE_ID, "create root page failed");

		UpdateRootPageId(root_page_id); // update the root page id in the header page
    

	}

	std::shared_ptr<BufferPoolManager> bpm_;
};
} // namespace db
