#pragma once

#include "buffer/buffer_pool_manager.hpp"
#include "common/rid.hpp"
#include "common/value.hpp"
#include "index/index.hpp"

#include <memory>
namespace db {

class BTreeIndex : public Index {
public:
	BTreeIndex(std::shared_ptr<IndexMeta> meta, std::shared_ptr<BufferPoolManager> bpm) : Index(meta), bpm_(bpm) {
	}

protected:
	bool InternalInsertRecord(const Value &key, const RID rid) override {
		return true;
	}
	bool InternalDeleteRecord(const Value &key) override {
		return true;
	}
	bool InternalScanKey(const Value &key, std::vector<RID> &rids) override {
		return true;
	}

private:
	std::shared_ptr<BufferPoolManager> bpm_;
	page_id_t header_page_id_ = 0;
};
} // namespace db
