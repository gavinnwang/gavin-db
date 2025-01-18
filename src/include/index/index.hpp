#pragma once

#include "common/logger.hpp"
#include "common/rid.hpp"
#include "common/typedef.hpp"
#include "common/value.hpp"
#include "concurrency/transaction.hpp"
#include "meta/column.hpp"
#include "storage/serializer/serializer.hpp"
#include "storage/table/table_meta.hpp"
#include "storage/table/tuple.hpp"

#include <sstream>

namespace db {

enum class IndexConstraintType : uint8_t {
	NONE = 0,    // no constraint
	UNIQUE = 1,  // built to enforce a UNIQUE constraint
	PRIMARY = 2, // built to enforce a PRIMARY KEY constraint
	FOREIGN = 3  // built to enforce a FOREIGN KEY constraint
};
enum class IndexType { BPlusTreeIndex, HashTableIndex };

struct IndexMeta {
public:
	IndexMeta() = default;
	IndexMeta(std::string name, table_oid_t table_id, Column key_col, IndexConstraintType index_constraint_type,
	          IndexType index_type)
	    : name_(std::move(name)), table_id_(table_id), key_col_(std::move(key_col)),
	      index_constraint_type_(index_constraint_type), index_type_(index_type) {
	}

	std::string name_;
	table_oid_t table_id_;
	index_oid_t index_id_;
	Column key_col_;
	IndexConstraintType index_constraint_type_;
	page_id_t header_page_id_ {INVALID_PAGE_ID};
	IndexType index_type_;

	void Serialize(Serializer &serializer) const {
		serializer.WriteProperty(100, "index_name", name_);
		serializer.WriteProperty(101, "table_id", table_id_);
		serializer.WriteProperty(102, "key_col", key_col_);
		serializer.WriteProperty(103, "index_id", index_id_);
		serializer.WriteProperty(104, "index_constraint_type", index_constraint_type_);
		serializer.WriteProperty(105, "header_page_id", header_page_id_);
		serializer.WriteProperty(106, "index_type", index_type_);
	}

	[[nodiscard]] static std::unique_ptr<IndexMeta> Deserialize(Deserializer &deserializer) {
		auto meta = std::make_unique<IndexMeta>();
		deserializer.ReadProperty(100, "table_name", meta->name_);
		deserializer.ReadProperty(101, "table_id", meta->table_id_);
		deserializer.ReadProperty(102, "key_col", meta->key_col_);
		deserializer.ReadProperty(103, "index_id", meta->index_id_);
		deserializer.ReadProperty(104, "index_constraint_type", meta->index_constraint_type_);
		deserializer.ReadProperty(105, "header_page_id", meta->header_page_id_);
		deserializer.ReadProperty(106, "index_type", meta->index_type_);
		return meta;
	}
};

// return 0 if a == b, 1 if a > b, -1 if a < b
using Comparator = std::function<int(const IndexKeyType &, const IndexKeyType &)>;

static int32_t ConvertArrayToInt32(const IndexKeyType &arr) {
	int32_t result = 0;
	// Assuming little-endian
	result |= arr[0];
	result |= arr[1] << 8;
	result |= arr[2] << 16;
	result |= arr[3] << 24;
	return result;
}

static std::string IndexKeyTypeToString(const IndexKeyType &key) {
	std::ostringstream oss;
	oss << "Key[";
	oss << ConvertArrayToInt32(key);
	oss << "]";
	return oss.str();
}
class Index : public PageAllocator {

public:
	Index() = delete;
	Index(const Index &) = delete;
	Index &operator=(const Index &) = delete;
	Index(IndexMeta &index_meta, TableMeta &table_meta)
	    : index_meta_(index_meta), table_meta_(table_meta), comparator_(GetComparator(index_meta.key_col_.GetType())) {
	}

	PageId AllocatePage() override {
		auto new_page = table_meta_.IncrementTableDataPageId();
		return {table_meta_.table_oid_, new_page};
	}
	bool InsertRecord(Transaction &txn, const Tuple &tuple, const RID rid) {
		auto key = ConvertTupleToKey(tuple);
		LOG_TRACE("Inserting key: %s", IndexKeyTypeToString(key).c_str());
		return InternalInsertRecord(txn, key, rid);
	}
	bool DeleteRecord(Transaction &txn, const Tuple &tuple) {
		auto key = ConvertTupleToKey(tuple);
		return InternalDeleteRecord(txn, key);
	}

	bool ScanKey(const Tuple &tuple, std::vector<RID> &rids) {
		auto key = ConvertTupleToKey(tuple);
		LOG_TRACE("Scanning key: %s", IndexKeyTypeToString(key).c_str());
		return InternalScanKey(key, rids);
	}

	~Index() override = default;

	// debug

protected:
	virtual bool InternalInsertRecord(Transaction &txn, IndexKeyType key, RID rid) = 0;
	virtual bool InternalDeleteRecord(Transaction &txn, IndexKeyType key) = 0;
	virtual bool InternalScanKey(IndexKeyType key, std::vector<RID> &rids) = 0;
	IndexMeta &index_meta_;
	TableMeta &table_meta_;
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

	[[nodiscard]] IndexKeyType ConvertTupleToKey(const Tuple &tuple) const {
		auto value = tuple.GetValue(index_meta_.key_col_);
		LOG_TRACE("converted to key: {}", value.ToString());
		return value.ConvertToIndexKeyType();
	}
};

} // namespace db
