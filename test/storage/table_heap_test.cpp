#include "buffer/buffer_pool_manager.hpp"
#include "common/fs_utils.hpp"
#include "common/test_utils.hpp"
#include "storage/table/table_heap.hpp"
#include "storage/table/tuple.hpp"

#include "gtest/gtest.h"

TEST(StorageTest, TableHeapSimpleTest) {

	db::DeletePathIfExists(db::FilePathManager::GetInstance().GetDatabaseRootPath());
	const size_t buffer_pool_size = 10;
	auto cm = std::make_shared<db::CatalogManager>();
	auto dm = std::make_shared<db::DiskManager>(cm);
	auto bpm = std::make_shared<db::BufferPoolManager>(buffer_pool_size, dm);

	auto c1 = db::Column("user_id", db::TypeId::INTEGER);
	auto c2 = db::Column("user_name", db::TypeId::VARCHAR, 256);
	auto schema = db::Schema({c1, c2});
	auto table_name = "usr";

	cm->CreateTable(table_name, schema);
	auto table_info = cm->GetTable(table_name);
	auto table_heap = new db::TableHeap(bpm, table_info);
	int32_t int_val = 2392;
	std::string str_val = "hhihi";
	auto v1 = db::Value(db::TypeId::INTEGER, int_val);
	auto v2 = db::Value(db::TypeId::VARCHAR, str_val);

	auto tuple = db::Tuple({v1, v2}, schema);

	auto meta = db::TupleMeta {false};
	auto rid1 = table_heap->InsertTuple(meta, tuple);
	auto rid2 = table_heap->InsertTuple(meta, tuple);
	auto rid3 = table_heap->InsertTuple(meta, tuple);

	ASSERT_EQ(rid1.has_value(), true);
	ASSERT_EQ(rid2.has_value(), true);
	ASSERT_EQ(rid3.has_value(), true);

	auto ret1 = table_heap->GetTuple(*rid1);
	auto ret2 = table_heap->GetTuple(*rid2);
	ASSERT_EQ(ret1.has_value(), true);
  ASSERT_EQ(ret2.has_value(), true);
	auto [meta1, tuple1] = *ret1;
	auto [meta2, tuple2] = *ret2;

	ASSERT_EQ(tuple.ToString(schema), "(" + std::to_string(int_val) + ", " + str_val + ")");
	ASSERT_EQ(tuple.ToString(schema), tuple1.ToString(schema));
	ASSERT_EQ(tuple.ToString(schema), tuple2.ToString(schema));
}

TEST(StorageTest, TableHeapManyInsertionTest) {
	db::DeletePathIfExists(db::FilePathManager::GetInstance().GetDatabaseRootPath());
	const size_t buffer_pool_size = 25;
	auto cm = std::make_shared<db::CatalogManager>();
	auto dm = std::make_shared<db::DiskManager>(cm);
	auto bpm = std::make_shared<db::BufferPoolManager>(buffer_pool_size, dm);

	// db::page_id_t table_info_page_id;
	// auto guard = bpm->NewPageGuarded(table_info_page_id);
	// ASSERT_NE(table_info_page_id, db::INVALID_PAGE_ID);

	auto c1 = db::Column("user_id", db::TypeId::INTEGER);
	auto c2 = db::Column("user_name", db::TypeId::VARCHAR, 256);
	auto schema = db::Schema({c1, c2});
	auto table_name = "usr";
	//
	// auto table_info_wpg = guard.UpgradeWrite();
	// auto table_info_page = table_info_wpg.AsMut<db::TableInfoPage>();
	// table_info_page->Init(table_name, schema, 0);
	// table_info_wpg.Drop();

	cm->CreateTable(table_name, schema);
	auto table_info = cm->GetTable(table_name);
	// auto table_info = std::make_shared<db::TableInfo>(schema, table_name, 0);
	auto table_heap = new db::TableHeap(bpm, table_info);

	std::vector<db::RID> rids;
	std::vector<std::string> ans;
	// insert 200,000 tuples
	for (int i = 0; i < 300000; ++i) {
		int32_t int_val = i;
		std::string str_val = db::GenerateRandomString(10, 256);

		auto v1 = db::Value(db::TypeId::INTEGER, int_val);
		auto v2 = db::Value(db::TypeId::VARCHAR, str_val);

		auto tuple = db::Tuple({v1, v2}, schema);
		auto meta = db::TupleMeta {false};

		auto rid = table_heap->InsertTuple(meta, tuple);
		ASSERT_EQ(rid.has_value(), true);
		rids.push_back(*rid);
		ans.push_back("(" + std::to_string(int_val) + ", " + std::move(str_val) + ")");
		std::cout << "rid: " << rid->GetPageId().page_number_ << " " << rid->GetSlotNum() << std::endl;
	}

	for (int i = 0; i < 300000; ++i) {
		auto ret = table_heap->GetTuple(rids[i]);
		if (!ret.has_value()) {
			std::cout << "Error: " << i << std::endl;
			continue;
		}
		auto [meta, tuple] = *ret;
		std::cout << tuple.ToString(schema) << std::endl;
		ASSERT_EQ(tuple.ToString(schema), ans[i]);
	}
}
