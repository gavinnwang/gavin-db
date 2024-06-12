#pragma once

#include "catalog/column.hpp"
#include "common/exception.hpp"
#include "common/logger.hpp"
#include "common/macros.hpp"
#include "common/rid.hpp"
#include "common/typedef.hpp"
#include "common/value.hpp"
#include "storage/serializer/serializer.hpp"
#include "storage/table/table_meta.hpp"
#include "storage/table/tuple.hpp"

#include <iomanip>
#include <sstream>

namespace db {
enum class IndexConstraintType : uint8_t {
	NONE = 0,    // no constraint
	UNIQUE = 1,  // built to enforce a UNIQUE constraint
	PRIMARY = 2, // built to enforce a PRIMARY KEY constraint
	FOREIGN = 3  // built to enforce a FOREIGN KEY constraint
};

struct IndexMeta : public PageAllocator {
public:
	explicit IndexMeta() = default;
	IndexMeta(std::string name, table_oid_t table_id, const std::shared_ptr<TableMeta> &table_meta, Column key_col,
	          IndexConstraintType index_constraint_type)
	    : name_(std::move(name)), table_id_(table_id), key_col_(std::move(key_col)),
	      index_constraint_type_(index_constraint_type), table_meta_(table_meta) {
	}

	std::string name_;
	table_oid_t table_id_;
	Column key_col_;
	// column_t key_col_id_;
	IndexConstraintType index_constraint_type_;
	page_id_t header_page_id_ {INVALID_PAGE_ID};
	const std::shared_ptr<TableMeta> table_meta_;

	PageId AllocatePage() {
		auto new_page = table_meta_->IncrementTableDataPageId();
		return {table_meta_->table_oid_, new_page};
	}

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
	oss << "IndexKeyType contents: [";
	oss << ConvertArrayToInt32(key);
	// for (size_t i = 0; i < key.size(); ++i) {
	// 	if (i != 0) {
	// 		oss << ", ";
	// 	}
	// 	// If data_t is not char, you might want to print in hexadecimal or as integer
	// 	if constexpr (std::is_same_v<data_t, char>) {
	// 		oss << key[i];
	// 	} else {
	// 		oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(key[i]);
	// 	}
	// }
	oss << "]";
	return oss.str();
}
class Index {

public:
	Index() = delete;
	DISALLOW_COPY(Index);
	Index(const std::shared_ptr<IndexMeta> &index_meta, std::shared_ptr<TableMeta> table_meta)
	    : index_meta_(index_meta), table_meta_(std::move(table_meta)),
	      comparator_(GetComparator(index_meta->key_col_.GetType())) {
	}

	bool InsertRecord(const Tuple &tuple, const RID rid) {
		auto key = ConvertTupleToKey(tuple);
		LOG_TRACE("Inserting key: %s", IndexKeyTypeToString(key).c_str());
		return InternalInsertRecord(key, rid);
	}
	bool DeleteRecord(const Tuple &tuple) {
		auto key = ConvertTupleToKey(tuple);
		return InternalDeleteRecord(key);
	}

	bool ScanKey(const Tuple &tuple, std::vector<RID> &rids) {
		auto key = ConvertTupleToKey(tuple);
		LOG_TRACE("Scanning key: %s", IndexKeyTypeToString(key).c_str());
		return InternalScanKey(key, rids);
	}

	virtual ~Index() = default;

	// debug

protected:
	virtual bool InternalInsertRecord(IndexKeyType key, RID rid) = 0;
	virtual bool InternalDeleteRecord(IndexKeyType key) = 0;
	virtual bool InternalScanKey(IndexKeyType key, std::vector<RID> &rids) = 0;
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
		// LOG_TRACE("Comparing %s and %s", IndexKeyTypeToString(a).c_str(), IndexKeyTypeToString(b).c_str());

		T lhs = *reinterpret_cast<const T *>(a.data());
		T rhs = *reinterpret_cast<const T *>(b.data());
		return (lhs < rhs) ? -1 : (lhs > rhs) ? 1 : 0;
	}
	template <>
	int Compare<std::string>(const IndexKeyType &a, const IndexKeyType &b) {
		(void)a;
		(void)b;
		throw NotImplementedException("varchar key? hmmm...");
		// return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end());
	}

	IndexKeyType ConvertTupleToKey(const Tuple &tuple) {
		auto value = tuple.GetValue(index_meta_->key_col_);
		LOG_TRACE("converted to key: %s", value.ToString().c_str());
		return value.ConvertToIndexKeyType();
	}
};

} // namespace db
