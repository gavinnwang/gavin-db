

#include "common/test_utils.hpp"
#include "index/btree_index.hpp"
#include "storage/table/table_heap.hpp"

#include "gtest/gtest.h"
#include <cstdint>
namespace db {

TEST(IndexTest, IndexTest) {
	const size_t buffer_pool_size = 10;
	auto cm = std::make_shared<db::CatalogManager>();
	auto dm = std::make_shared<db::DiskManager>(cm);
	auto bpm = std::make_shared<db::BufferPoolManager>(buffer_pool_size, dm);

	auto c1 = db::Column("user_id", db::TypeId::INTEGER);
	auto c2 = db::Column("user_name", db::TypeId::VARCHAR, 256);
	auto schema = db::Schema({c1, c2});
	const auto *table_name = "user";

	cm->CreateTable(table_name, schema);
	auto table_meta = cm->GetTable(table_name);

	auto index_meta = std::make_shared<IndexMeta>("user_id_index", table_meta->table_oid_, table_meta,
	                                              schema.GetColumn(0), IndexConstraintType::PRIMARY);

	auto btree_index = std::make_unique<BTreeIndex>(index_meta, table_meta, bpm);

	int32_t int_val = 1;
	std::string str_val = "hi";

	auto table_heap = std::make_unique<TableHeap>(bpm, table_meta);

	auto v1 = db::Value(db::TypeId::INTEGER, int_val);
	auto v2 = db::Value(db::TypeId::VARCHAR, str_val);

	auto tuple = db::Tuple({v1, v2}, schema);
	int32_t int_val_2 = 2;
	std::string str_val_2 = "yo";
	auto v3 = db::Value(db::TypeId::INTEGER, int_val_2);
	auto v4 = db::Value(db::TypeId::VARCHAR, str_val_2);
	auto tuple2 = db::Tuple({v3, v4}, schema);
	auto meta = db::TupleMeta {false};

	auto rid = table_heap->InsertTuple(meta, tuple);

	if (rid.has_value()) {
		auto res = btree_index->InsertRecord(tuple, *rid);
		assert(res);
		std::vector<RID> rids;
		btree_index->ScanKey(tuple, rids);
		ASSERT_EQ(rids.size(), 1);
		LOG_DEBUG("rids returneed %d %d", rids[0].GetPageId().page_number_, rids[0].GetSlotNum());
	}

	rid = table_heap->InsertTuple(meta, tuple2);

	if (rid.has_value()) {
		LOG_TRACE("tuple %s", tuple.ToString(schema).c_str());

		std::cout << (*rid).GetPageId().page_number_ << (*rid).GetSlotNum() << std::endl;

		LOG_TRACE("tuple %s", tuple2.ToString(schema).c_str());
		auto res = btree_index->InsertRecord(tuple2, *rid);
		assert(res);
		std::vector<RID> rids;
		btree_index->ScanKey(tuple2, rids);
		ASSERT_EQ(rids.size(), 1);
		LOG_DEBUG("rids returneed %d %d", rids[0].GetPageId().page_number_, rids[0].GetSlotNum());
	}
}

TEST(IndexTest, IndexManyInsertionsTest) {
	const size_t buffer_pool_size = 30;
	auto cm = std::make_shared<db::CatalogManager>();
	auto dm = std::make_shared<db::DiskManager>(cm);
	auto bpm = std::make_shared<db::BufferPoolManager>(buffer_pool_size, dm);

	auto c1 = db::Column("user_id", db::TypeId::INTEGER);
	auto c2 = db::Column("user_name", db::TypeId::VARCHAR, 256);
	auto schema = db::Schema({c1, c2});
	const auto *table_name = "user";

	cm->CreateTable(table_name, schema);
	auto table_meta = cm->GetTable(table_name);

	auto index_meta = std::make_shared<IndexMeta>("user_id_index", table_meta->table_oid_, table_meta,
	                                              schema.GetColumn(0), IndexConstraintType::PRIMARY);

	auto btree_index = std::make_unique<BTreeIndex>(index_meta, table_meta, bpm);

	int32_t int_val = 1;
	std::string str_val = "hi";

	auto table_heap = std::make_unique<TableHeap>(bpm, table_meta);

	auto v1 = db::Value(db::TypeId::INTEGER, int_val);
	auto v2 = db::Value(db::TypeId::VARCHAR, str_val);

	auto tuple = db::Tuple({v1, v2}, schema);
	int32_t int_val_2 = 2;
	std::string str_val_2 = "yo";
	auto v3 = db::Value(db::TypeId::INTEGER, int_val_2);
	auto v4 = db::Value(db::TypeId::VARCHAR, str_val_2);
	auto tuple2 = db::Tuple({v3, v4}, schema);

	std::vector<RID> rids;
	std::vector<std::string> ans;
	std::vector<Tuple> tuples;

	constexpr int n = 3000;
	for (int i = 0; i < n; ++i) {
		int32_t int_val = i;
		std::string str_val = GenerateRandomString(10, 256);

		auto v1 = Value(db::TypeId::INTEGER, int_val);
		auto v2 = Value(db::TypeId::VARCHAR, str_val);

		auto tuple = Tuple({v1, v2}, schema);
		auto meta = TupleMeta {false};

		auto rid = table_heap->InsertTuple(meta, tuple);

		btree_index->InsertRecord(tuple, *rid);
		tuples.push_back(tuple);

		ASSERT_EQ(rid.has_value(), true);
		rids.push_back(*rid);
		ans.push_back("(" + std::to_string(int_val) + ", " + std::move(str_val) + ")");
		LOG_DEBUG("rid: %d %d", rid->GetPageId().page_number_, rid->GetSlotNum());
	}

	for (int i = 0; i < n; ++i) {
		auto ret = table_heap->GetTuple(rids[i]);
		std::vector<RID> scan_ans;
		auto res = btree_index->ScanKey(tuples[i], scan_ans);
		assert(res);
		LOG_DEBUG("scan ans size %lu", scan_ans.size());

		if (!ret.has_value()) {
			LOG_ERROR("tuple not found");
			continue;
		}
		auto [meta, tuple] = *ret;
		// LOG_DEBUG("tuple: %s", tuple.ToString(schema).c_str());
		ASSERT_EQ(tuple.ToString(schema), ans[i]);
	}
}
} // namespace db
