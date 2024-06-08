#pragma once

#include "buffer/buffer_pool_manager.hpp"
#include "catalog/column.hpp"
#include "catalog/schema.hpp"
#include "common/macros.hpp"
#include "common/rid.hpp"
#include "common/typedef.hpp"
#include "common/value.hpp"
#include "storage/serializer/serializer.hpp"
#include "storage/table/tuple.hpp"

#include <unordered_set>
namespace db {
enum class IndexConstraintType : uint8_t {
	NONE = 0,    // no constraint
	UNIQUE = 1,  // built to enforce a UNIQUE constraint
	PRIMARY = 2, // built to enforce a PRIMARY KEY constraint
	FOREIGN = 3  // built to enforce a FOREIGN KEY constraint
};

struct IndexMeta {
public:
  explicit IndexMeta() = default;
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
  page_id_t header_page_id_ = 0;

  void Serialize(Serializer &serializer) const {
    serializer.WriteProperty(100, "index_name", name_);
    serializer.WriteProperty(101, "table_id", table_id_);
    serializer.WriteProperty(102, "key_col", key_col_);
    serializer.WriteProperty(103, "key_col_id", key_col_id_);
    serializer.WriteProperty(104, "index_constraint_type", index_constraint_type_);
    serializer.WriteProperty(105, "header_page_id", header_page_id_);
  }
  
  [[nodiscard]] static std::unique_ptr<IndexMeta> Deserialize(Deserializer &deserializer) {
    auto meta  = std::make_unique<IndexMeta>();
    deserializer.ReadProperty(100, "table_name", meta->name_);
    deserializer.ReadProperty(101, "table_id", meta->table_id_);
    deserializer.ReadProperty(102, "key_col", meta->key_col_);
    deserializer.ReadProperty(103, "key_col_id", meta->key_col_id_);
    deserializer.ReadProperty(104, "index_constraint_type", meta->index_constraint_type_);
    deserializer.ReadProperty(105, "header_page_id", meta->header_page_id_);
    return meta;
  }

};

class Index {

public:
	Index() = delete;
  DISALLOW_COPY(Index);
	Index(std::shared_ptr<IndexMeta> index_meta, std::shared_ptr<TableMeta> table_meta) : index_meta_(index_meta), table_meta_(table_meta) 
{}

	bool InsertRecord(const Tuple &tuple, const RID rid) {
		auto key = ConvertTupleToKey(tuple);
		return InternalInsertRecord(std::move(key), rid);
	}
	bool DeleteRecord(const Tuple &tuple) {
		auto key = ConvertTupleToKey(tuple);
		return InternalDeleteRecord(std::move(key));
	}

	bool ScanKey(const Tuple &tuple, std::vector<RID> &rids) {
		auto key = ConvertTupleToKey(tuple);
		return InternalScanKey(std::move(key), rids);
	}

protected:
	virtual bool InternalInsertRecord(const IndexKeyType key, const RID rid) = 0;
	virtual bool InternalDeleteRecord(const IndexKeyType key) = 0;
	virtual bool InternalScanKey(const IndexKeyType key, std::vector<RID> &rids) = 0;
	std::shared_ptr<IndexMeta> index_meta_;
  std::shared_ptr<TableMeta> table_meta_;
private:
	const IndexKeyType ConvertTupleToKey(const Tuple &tuple) {
		return tuple.GetValue(index_meta_->key_col_).ConvertToIndexKeyType();
	}
};

} // namespace db
