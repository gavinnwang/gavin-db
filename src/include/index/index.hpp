#pragma once

#include "buffer/buffer_pool_manager.hpp"
#include "catalog/column.hpp"
#include "catalog/schema.hpp"
#include "common/macros.hpp"
#include "common/rid.hpp"
#include "common/typedef.hpp"
#include "common/value.hpp"
#include "storage/page/btree_page.hpp"
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
	IndexMeta(const std::string &name, table_oid_t table_id, const Column key_col,
	          IndexConstraintType index_constraint_type)
	    : name_(name), table_id_(table_id), key_col_(std::move(key_col)),
	      index_constraint_type_(index_constraint_type) {
	}

	std::string name_;
	table_oid_t table_id_;
	Column key_col_;
	// column_t key_col_id_;
	IndexConstraintType index_constraint_type_;
	page_id_t header_page_id_ {INVALID_PAGE_ID};

	void Serialize(Serializer &serializer) const {
		serializer.WriteProperty(100, "index_name", name_);
		serializer.WriteProperty(101, "table_id", table_id_);
		serializer.WriteProperty(102, "key_col", key_col_);
		// serializer.WriteProperty(103, "key_col_id", key_col_id_);
		serializer.WriteProperty(104, "index_constraint_type", index_constraint_type_);
		serializer.WriteProperty(105, "header_page_id", header_page_id_);
	}

	[[nodiscard]] static std::unique_ptr<IndexMeta> Deserialize(Deserializer &deserializer) {
		auto meta = std::make_unique<IndexMeta>();
		deserializer.ReadProperty(100, "table_name", meta->name_);
		deserializer.ReadProperty(101, "table_id", meta->table_id_);
		deserializer.ReadProperty(102, "key_col", meta->key_col_);
		// deserializer.ReadProperty(103, "key_col_id", meta->key_col_id_);
		deserializer.ReadProperty(104, "index_constraint_type", meta->index_constraint_type_);
		deserializer.ReadProperty(105, "header_page_id", meta->header_page_id_);
		return meta;
	}
};

using Comparator = std::function<int(const IndexKeyType &, const IndexKeyType &)>;
class Index {

public:
	Index() = delete;
	DISALLOW_COPY(Index);
	Index(std::shared_ptr<IndexMeta> index_meta, std::shared_ptr<TableMeta> table_meta)
	    : index_meta_(index_meta), table_meta_(table_meta) {
	}

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

  virtual ~Index() = default;

protected:
	virtual bool InternalInsertRecord(const IndexKeyType key, const RID rid) = 0;
	virtual bool InternalDeleteRecord(const IndexKeyType key) = 0;
	virtual bool InternalScanKey(const IndexKeyType key, std::vector<RID> &rids) = 0;
	std::shared_ptr<IndexMeta> index_meta_;
	std::shared_ptr<TableMeta> table_meta_;
	// comparator used to determine the order of keys
	Comparator comparator_;

private:
	static Comparator GetComparator(TypeId type_id) {
		switch (type_id) {
		case TypeId::BOOLEAN:
			return Compare<uint8_t>;
		case TypeId::INTEGER:
			return Compare<int32_t>;
		case TypeId::TIMESTAMP:
			return Compare<uint64_t>;
		case TypeId::VARCHAR:
			return Compare<std::string>;
		default:
			throw std::invalid_argument("Unsupported type for indexing");
		}
	}

	template <typename T>
	static int Compare(const IndexKeyType &a, const IndexKeyType &b) {
		T lhs = *reinterpret_cast<const T *>(a.data());
		T rhs = *reinterpret_cast<const T *>(b.data());
		return (lhs < rhs) ? -1 : (lhs > rhs) ? 1 : 0;
	}
	template <>
	int Compare<std::string>(const IndexKeyType &a, const IndexKeyType &b) {
		return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end());
	}

	const IndexKeyType ConvertTupleToKey(const Tuple &tuple) {
		return tuple.GetValue(index_meta_->key_col_).ConvertToIndexKeyType();
	}
};

} // namespace db
