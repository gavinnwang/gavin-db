

#include "index/btree_index.hpp"
#include "storage/table/table_heap.hpp"

#include "gtest/gtest.h"
namespace db {

TEST(IndexTest, IndexTest) {

	// BTreeIndex(std::shared_ptr<IndexMeta> index_meta, std::shared_ptr<TableMeta> table_meta,
	//            std::shared_ptr<BufferPoolManager> bpm)
	const size_t buffer_pool_size = 10;
	auto cm = std::make_shared<db::CatalogManager>();
	auto dm = std::make_shared<db::DiskManager>(cm);
	auto bpm = std::make_shared<db::BufferPoolManager>(buffer_pool_size, dm);

	auto c1 = db::Column("user_id", db::TypeId::INTEGER);
	auto c2 = db::Column("user_name", db::TypeId::VARCHAR, 256);
	// std::cout << c1.ToString(false) << std::endl;
	// std::cout << c2.ToString(false);
	auto schema = db::Schema({c1, c2});
	// std::cout << c1.ToString(false) << std::endl;
	// std::cout << c2.ToString(false);
	auto table_name = "user";

	auto table_oid = cm->TryCreateTable(table_name, schema);
	auto table_meta = cm->GetTable(table_name);

	auto index_meta =
	    std::make_shared<IndexMeta>("user_id_index", table_oid, schema.GetColumn(0), IndexConstraintType::PRIMARY);

	auto btree_index = new BTreeIndex(index_meta, table_meta, bpm);

	uint32_t int_val = 1;
	std::string str_val = "hi";

	auto table_heap = new TableHeap(bpm, table_meta);

	auto v1 = db::Value(db::TypeId::INTEGER, int_val);
	auto v2 = db::Value(db::TypeId::VARCHAR, str_val);

	auto tuple = db::Tuple({v1, v2}, schema);
	auto meta = db::TupleMeta {false};

	auto rid = table_heap->InsertTuple(meta, tuple);
	if (rid.has_value()) {
		btree_index->InsertRecord(tuple, *rid);
	}
}
} // namespace db
