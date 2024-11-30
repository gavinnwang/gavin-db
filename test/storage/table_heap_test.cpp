#include "buffer/buffer_pool_manager.hpp"
#include "catalog/catalog_manager.hpp"
#include "common/fs_utils.hpp"
#include "common/logger.hpp"
#include "common/test_utils.hpp"
#include "common/value.hpp"
#include "storage/table/table_heap.hpp"
#include "storage/table/tuple.hpp"

#include "gtest/gtest.h"

namespace db {

TEST(StorageTest, TableHeapSimpleTest) {

	DeletePathIfExists(db::FilePathManager::GetInstance().GetDatabaseRootPath());
	const size_t buffer_pool_size = 10;
	auto cm = std::make_unique<CatalogManager>();
	auto dm = std::make_shared<DiskManager>(cm);
	auto bpm = std::make_unique<BufferPoolManager>(buffer_pool_size, dm);

	auto c1 = Column("user_id", db::TypeId::INTEGER);
	auto c2 = Column("user_name", db::TypeId::VARCHAR, 256);
	auto schema = Schema({c1, c2});
	auto table_name = "user";

	cm->CreateTable(table_name, schema);
	auto &table_meta = cm->GetTable(table_name);
	auto table_heap = std::make_unique<TableHeap>(bpm, table_meta);
	int32_t int_val = 2392;
	std::string str_val = "hhihi";
	auto v1 = Value(db::TypeId::INTEGER, int_val);
	auto v2 = Value(db::TypeId::VARCHAR, str_val);

	auto tuple = Tuple({v1, v2}, schema);

	auto meta = TupleMeta {false};
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
	DeletePathIfExists(db::FilePathManager::GetInstance().GetDatabaseRootPath());
	const size_t buffer_pool_size = 25;
	auto cm = std::make_unique<CatalogManager>();
	auto dm = std::make_shared<DiskManager>(cm);
	auto bpm = std::make_unique<BufferPoolManager>(buffer_pool_size, dm);

	auto c1 = Column("user_id", db::TypeId::INTEGER);
	auto c2 = Column("user_name", db::TypeId::VARCHAR, 256);
	auto schema = Schema({c1, c2});
	auto table_name = "user";

	cm->CreateTable(table_name, schema);
	auto &table_meta = cm->GetTable(table_name);
	auto table_heap = std::make_unique<TableHeap>(bpm, table_meta);

	std::vector<RID> rids;
	std::vector<std::string> ans;
	// insert 200,000 tuples
	for (int i = 0; i < 300000; ++i) {
		int32_t int_val = i;
		std::string str_val = GenerateRandomString(10, 256);

		auto v1 = Value(db::TypeId::INTEGER, int_val);
		auto v2 = Value(db::TypeId::VARCHAR, str_val);

		auto tuple = Tuple({v1, v2}, schema);
		auto meta = TupleMeta {false};

		auto rid = table_heap->InsertTuple(meta, tuple);
		ASSERT_EQ(rid.has_value(), true);
		rids.push_back(*rid);
		ans.push_back("(" + std::to_string(int_val) + ", " + std::move(str_val) + ")");
		LOG_DEBUG("rid: %d %d", rid->GetPageId().page_number_, rid->GetSlotNum());
	}

	for (int i = 0; i < 300000; ++i) {
		auto ret = table_heap->GetTuple(rids[i]);
		if (!ret.has_value()) {
			LOG_ERROR("tuple not found");
			continue;
		}
		auto [meta, tuple] = *ret;
		LOG_DEBUG("tuple: %s", tuple.ToString(schema).c_str());
		ASSERT_EQ(tuple.ToString(schema), ans[i]);
	}
}
} // namespace db
