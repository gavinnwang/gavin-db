#include "buffer/buffer_pool_manager.hpp"
#include "common/fs_utils.hpp"
#include "storage/table_heap.hpp"
#include "storage/table_info_page.hpp"
#include "storage/tuple.hpp"

#include "gtest/gtest.h"
#include <cstdint>
#include <random>

TEST(StorageTest, TableHeapSimpleTest) {
  
	db::DeletePathIfExists(db::FilePathManager::GetInstance().GetDatabaseRootPath());
	const size_t buffer_pool_size = 10;
	auto cm = std::make_shared<db::CatalogManager>();
	auto dm = std::make_shared<db::DiskManager>(cm);
	auto bpm = std::make_shared<db::BufferPoolManager>(buffer_pool_size, dm, cm);
	// db::page_id_t table_info_page_id;
	// auto guard = bpm->NewPageGuarded(table_info_page_id);
	// ASSERT_NE(table_info_page_id, db::INVALID_PAGE_ID);

	auto c1 = db::Column("user_id", db::TypeId::INTEGER);
	auto c2 = db::Column("user_name", db::TypeId::VARCHAR, 256);
	auto schema = db::Schema({c1, c2});
	auto table_name = "usr";

	// auto table_info_wpg = guard.UpgradeWrite();
	// auto table_info_page = table_info_wpg.AsMut<db::TableInfoPage>();
	// table_info_page->Init(table_name, schema, 0);
	// table_info_wpg.Drop();

	cm->CreateTable(table_name, schema);
	auto table_info = cm->GetTable(table_name);
	auto table_heap = new db::TableHeap(bpm, table_info);
	int32_t int_val = 2392;
	std::string str_val = "hhihi";
	auto v1 = db::Value(db::TypeId::INTEGER, int_val);
	auto v2 = db::Value(db::TypeId::VARCHAR, str_val);
	// auto v2 = db::Value(db::TypeId::VARCHAR, str_val.data(),
	//                     static_cast<uint32_t>(str_val.size()), true);

	auto tuple = db::Tuple({v1, v2}, schema);

	auto meta = db::TupleMeta {false};
	auto rid1 = table_heap->InsertTuple(meta, tuple);
	auto rid2 = table_heap->InsertTuple(meta, tuple);
	auto rid3 = table_heap->InsertTuple(meta, tuple);

	ASSERT_EQ(rid1.has_value(), true);
	ASSERT_EQ(rid2.has_value(), true);
	ASSERT_EQ(rid3.has_value(), true);

	auto [meta1, tuple1] = table_heap->GetTuple(*rid1);
	auto [meta2, tuple2] = table_heap->GetTuple(*rid2);

	ASSERT_EQ(tuple.ToString(schema), "(" + std::to_string(int_val) + ", " + str_val + ")");
	ASSERT_EQ(tuple.ToString(schema), tuple1.ToString(schema));
	ASSERT_EQ(tuple.ToString(schema), tuple2.ToString(schema));
}
std::string generateRandomString(int a, int b) {
	// Seed with a real random value, if available
	std::random_device rd;

	// Choose a random length between a and b (inclusive)
	std::mt19937 gen(rd()); // Standard mersenne_twister_engine
	std::uniform_int_distribution<> dist(a, b);
	int stringLength = dist(gen);

	// Define a character set to use in the string
	const char charset[] = "0123456789"
	                       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	                       "abcdefghijklmnopqrstuvwxyz";
	int charsetSize = sizeof(charset) - 1; // Subtract 1 to avoid the null terminator

	// Create a random string
	std::string randomString;
	randomString.reserve(stringLength);

	std::uniform_int_distribution<> charDist(0, charsetSize - 1);
	for (int i = 0; i < stringLength; ++i) {
		randomString += charset[charDist(gen)];
	}

	return randomString;
}

TEST(StorageTest, TableHeapManyInsertionTest) {
	db::DeletePathIfExists(db::FilePathManager::GetInstance().GetDatabaseRootPath());
	const size_t buffer_pool_size = 25;
	auto cm = std::make_shared<db::CatalogManager>();
	auto dm = std::make_shared<db::DiskManager>(cm);
	auto bpm = std::make_shared<db::BufferPoolManager>(buffer_pool_size, dm, cm);

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
	for (int i = 0; i < 200000; ++i) {
		int32_t int_val = i;
		std::string str_val = generateRandomString(10, 256);

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

	for (int i = 0; i < 200000; ++i) {
		auto [meta, tuple] = table_heap->GetTuple(rids[i]);
		std::cout << tuple.ToString(schema) << std::endl;
		ASSERT_EQ(tuple.ToString(schema), ans[i]);
	}
}
