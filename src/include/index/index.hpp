#pragma once

#include "buffer/buffer_pool_manager.hpp"
#include "catalog/column.hpp"
#include "catalog/schema.hpp"
#include "common/macros.hpp"
#include "common/rid.hpp"
#include "common/typedef.hpp"
#include "storage/table/tuple.hpp"

#include <unordered_set>
namespace db {
enum class IndexConstraintType : uint8_t {
	NONE = 0,    // no constraint
	UNIQUE = 1,  // built to enforce a UNIQUE constraint
	PRIMARY = 2, // built to enforce a PRIMARY KEY constraint
	FOREIGN = 3  // built to enforce a FOREIGN KEY constraint
};

class IndexMeta {
public:
	IndexMeta(const std::string &name, table_oid_t table_id, const Column key_col, const column_t key_col_id,
	          IndexConstraintType index_constraint_type)
	    : name_(name), table_id_(table_id), key_col_(key_col), key_col_id_(key_col_id),
	      index_constraint_type_(index_constraint_type) {
	}

	std::string name_;
	table_oid_t table_id_;
	Column key_col_;
	column_t key_col_id_;
	IndexConstraintType index_constraint_type_;
};

class Index {

public:
	Index() = default;
	DISALLOW_COPY_AND_MOVE(Index);
	Index(std::shared_ptr<IndexMeta> meta) : meta_(meta) {
	}

	bool InsertRecord(const Tuple &tuple, const RID rid) {
		auto key = ConvertTupleToKey(tuple);
		return InternalInsertRecord(key, rid);
	}
	bool DeleteRecord(const Tuple &tuple) {
		auto key = ConvertTupleToKey(tuple);
		return InternalDeleteRecord(key);
	}

	bool ScanKey(const Tuple &tuple, std::vector<RID> &rids) {
		auto key = ConvertTupleToKey(tuple);
		return InternalScanKey(key, rids);
	}

protected:
	virtual bool InternalInsertRecord(const Value &key, const RID rid) = 0;
	virtual bool InternalDeleteRecord(const Value &key) = 0;
	virtual bool InternalScanKey(const Value &key, std::vector<RID> &rids) = 0;

private:
	Value ConvertTupleToKey(const Tuple &tuple) {
		return Value(tuple.GetValue(meta_->key_col_));
	}
	std::shared_ptr<IndexMeta> meta_;
};

} // namespace db
