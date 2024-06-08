#pragma once

#include "buffer/buffer_pool_manager.hpp"
#include "common/rid.hpp"
#include "common/typedef.hpp"
#include "common/value.hpp"
#include "index/index.hpp"
#include "storage/page/btree_leaf_page.hpp"

#include <memory>
namespace db {

class BTreeIndex : public Index {
public:
	BTreeIndex(std::shared_ptr<IndexMeta> index_meta, std::shared_ptr<TableMeta> table_meta, std::shared_ptr<BufferPoolManager> bpm) : Index(index_meta, table_meta), bpm_(bpm) {
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

  void Init();
  
  void CreateNewRoot(const IndexKeyType &key, const IndexValueType& value) {
    auto root_page_id = PageId{table_meta_->table_oid_};
    auto leaf_wpg = bpm_->NewPageGuarded(*table_meta_, root_page_id).UpgradeWrite();
    auto leaf_page = leaf_wpg.AsMut<BtreeLeafPage>();
    leaf_page->Init();
  }


	std::shared_ptr<BufferPoolManager> bpm_;
};
} // namespace db
