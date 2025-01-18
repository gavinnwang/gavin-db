

#include "meta/catalog.hpp"
#include "common/test_utils.hpp"
#include "concurrency/transaction.hpp"
#include "index/bplus_tree_index.hpp"
#include "index/index.hpp"
#include "storage/table/table_heap.hpp"

#include "gtest/gtest.h"
#include <cstdint>
namespace db {

TEST(IndexTest, IndexTest) {
	const size_t buffer_pool_size = 10;
	auto cm = std::make_unique<Catalog>();
	auto dm = std::make_shared<DiskManager>(cm);
	auto bpm = std::make_unique<BufferPool>(buffer_pool_size, dm);

	auto c1 = db::Column("user_id", db::TypeId::INTEGER);
	auto c2 = db::Column("user_name", db::TypeId::VARCHAR, 256);
	auto schema = db::Schema({c1, c2});
	const auto *table_name = "user";

	cm->CreateTable(table_name, schema);
	auto &table_meta = cm->GetTableByName(table_name);

	auto index_meta = std::make_unique<IndexMeta>("user_id_index", table_meta.table_oid_, schema.GetColumn(0),
	                                              IndexConstraintType::PRIMARY, IndexType::BPlusTreeIndex);

	auto btree_index = std::make_unique<BTreeIndex>(*index_meta, table_meta, *bpm);

	int32_t int_val = 1;
	std::string str_val = "hi";

	auto table_heap = std::make_unique<TableHeap>(*bpm, table_meta);

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

	Transaction txn {1, IsolationLevel::READ_UNCOMMITTED};
	if (rid.has_value()) {
		auto res = btree_index->InsertRecord(txn, tuple, *rid);
		assert(res);
		std::vector<RID> rids;
		btree_index->ScanKey(tuple, rids);
		ASSERT_EQ(rids.size(), 1);
		LOG_DEBUG("rids returneed %d %d", rids[0].GetPageId().page_number_, rids[0].GetSlotNum());
	}

	rid = table_heap->InsertTuple(meta, tuple2);

	if (rid.has_value()) {
		LOG_TRACE("tuple %s", tuple.ToString(schema).c_str());
		LOG_TRACE("{}", rid->ToString());

		LOG_TRACE("tuple %s", tuple2.ToString(schema).c_str());
		auto res = btree_index->InsertRecord(txn, tuple2, *rid);
		assert(res);
		std::vector<RID> rids;
		btree_index->ScanKey(tuple2, rids);
		ASSERT_EQ(rids.size(), 1);
		LOG_DEBUG("rids returneed %d %d", rids[0].GetPageId().page_number_, rids[0].GetSlotNum());
	}
}

TEST(IndexTest, IndexManyInsertionsTest) {
	const size_t buffer_pool_size = 13;
	auto cm = std::make_unique<db::Catalog>();
	auto dm = std::make_shared<db::DiskManager>(cm);
	auto bpm = std::make_unique<db::BufferPool>(buffer_pool_size, dm);

	auto c1 = db::Column("user_id", db::TypeId::INTEGER);
	auto c2 = db::Column("user_name", db::TypeId::VARCHAR, 64);
	auto schema = db::Schema({c1, c2});
	const auto *table_name = "user";

	cm->CreateTable(table_name, schema);
	auto &table_meta = cm->GetTableByName(table_name);

	auto index_meta = std::make_unique<IndexMeta>("user_id_index", table_meta.table_oid_, schema.GetColumn(0),
	                                              IndexConstraintType::PRIMARY, IndexType::BPlusTreeIndex);

	auto btree_index = std::make_unique<BTreeIndex>(*index_meta, table_meta, *bpm);

	int32_t int_val = 1;
	std::string str_val = "hi";

	auto table_heap = std::make_unique<TableHeap>(*bpm, table_meta);

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

	constexpr int n = 15000;
	Transaction txn {1, IsolationLevel::READ_UNCOMMITTED};
	for (int i = 0; i < n; ++i) {
		int32_t int_val = i;
		std::string str_val = GenerateRandomString(1, 50);

		auto v1 = Value(db::TypeId::INTEGER, int_val);
		auto v2 = Value(db::TypeId::VARCHAR, str_val);

		auto tuple = Tuple({v1, v2}, schema);
		auto meta = TupleMeta {false};

		auto rid = table_heap->InsertTuple(meta, tuple);

		btree_index->InsertRecord(txn, tuple, *rid);
		tuples.push_back(tuple);

		ASSERT_EQ(rid.has_value(), true);
		rids.push_back(*rid);
		ans.push_back("(" + std::to_string(int_val) + ", " + std::move(str_val) + ")");
		LOG_DEBUG("rid  %d %d", rid->GetPageId().page_number_, rid->GetSlotNum());
	}

	for (int i = 0; i < n; ++i) {
		std::vector<RID> scan_ans;
		auto res = btree_index->ScanKey(tuples[i], scan_ans);
		LOG_DEBUG("Scanned %s", tuples[i].ToString(schema).c_str());
		LOG_DEBUG("%d scan ans size %lu", res, scan_ans.size());

		assert(res);
		if (res) {
			ASSERT_EQ(scan_ans.size(), 1);
			auto rid = scan_ans[0];
			LOG_DEBUG("rid %d %d", rid.GetPageId().page_number_, rid.GetSlotNum());
			ASSERT_EQ(rids[i], rid);
		}
	}
}
} // namespace db
