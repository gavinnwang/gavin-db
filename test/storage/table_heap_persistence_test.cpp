#include "buffer/buffer_pool_manager.hpp"
#include "common/fs_utils.hpp"
#include "common/test_utils.hpp"
#include "common/typedef.hpp"
#include "common/value.hpp"
#include "storage/table/table_heap.hpp"
#include "storage/table/tuple.hpp"

#include "gtest/gtest.h"
#include <cstdint>
#include <vector>

TEST(StorageTest, TablePersistTest) {
	// db::DeletePathIfExists(db::FilePathManager::GetInstance().GetDatabaseRootPath());
	const size_t buffer_pool_size = 10;
	auto cm = std::make_shared<db::CatalogManager>();
	auto dm = std::make_shared<db::DiskManager>(cm);
	auto bpm = std::make_shared<db::BufferPoolManager>(buffer_pool_size, dm);

	auto c1 = db::Column("user_id", db::TypeId::INTEGER);
	auto c2 = db::Column("user_name", db::TypeId::VARCHAR, 256);
	auto schema = db::Schema({c1, c2});
	auto table_name = "usr";

	cm->TryCreateTable(table_name, schema);
	auto table_meta = cm->GetTable(table_name);
	auto table_heap = new db::TableHeap(bpm, table_meta);

	std::vector<db::RID> rids;
	for (int i = 0; i < 200; ++i) {
		int32_t int_val = i;
		std::string str_val = db::GenerateRandomString(10, 256);

		auto v1 = db::Value(db::TypeId::INTEGER, int_val);
		auto v2 = db::Value(db::TypeId::VARCHAR, str_val);

		auto tuple = db::Tuple({v1, v2}, schema);
		auto meta = db::TupleMeta {false};

		auto rid = table_heap->InsertTuple(meta, tuple);
		ASSERT_EQ(rid.has_value(), true);
		rids.push_back(*rid);
		// ans.push_back("(" + std::to_string(int_val) + ", " + std::move(str_val) + ")");
		std::cout << "rid: " << rid->GetPageId().page_number_ << " " << rid->GetSlotNum() << std::endl;
	}

	/// flush pages to disk
	bpm->FlushAllPages();
	cm->PersistToDisk();

	auto cm2 = std::make_shared<db::CatalogManager>();
	auto dm2 = std::make_shared<db::DiskManager>(cm);
	auto bpm2 = std::make_shared<db::BufferPoolManager>(buffer_pool_size, dm);

	auto table_meta2 = cm2->GetTable(table_name);
	std::cout << table_meta2->name_ << std::endl;

	auto table_heap2 = new db::TableHeap(bpm2, table_meta2);

	for (int i = 0; i < 200; ++i) {
		auto ret = table_heap2->GetTuple(rids[i]);
		if (!ret.has_value()) {
			std::cout << "Error: " << i << std::endl;
			continue;
		}
		auto [meta, tuple] = *ret;
		std::cout << tuple.ToString(schema) << std::endl;
		// ASSERT_EQ(tuple.ToString(schema), ans[i]);
	}
	auto it = table_heap2->MakeIterator();
	while (!it.IsEnd()) {
		auto [meta, tuple] = *it.GetTuple();
		std::cout << tuple.ToString(schema) << std::endl;
		++it;
	}
}
