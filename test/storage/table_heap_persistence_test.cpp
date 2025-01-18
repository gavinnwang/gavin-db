#include "common/logger.hpp"
#include "common/test_utils.hpp"
#include "common/typedef.hpp"
#include "common/value.hpp"
#include "meta/catalog.hpp"
#include "storage/buffer/buffer_pool.hpp"
#include "storage/table/table_heap.hpp"
#include "storage/table/tuple.hpp"

#include "gtest/gtest.h"
#include <cstdint>
#include <vector>

namespace db {

TEST(StorageTest, TablePersistTest) {
	// DeletePathIfExists(db::FilePathManager::GetInstance().GetDatabaseRootPath());
	const size_t buffer_pool_size = 10;
	auto cm = std::make_unique<Catalog>();
	auto dm = std::make_unique<DiskManager>(*cm);
	auto bpm = std::make_unique<BufferPool>(buffer_pool_size, *dm);

	auto c1 = Column("user_id", db::TypeId::INTEGER);
	auto c2 = Column("user_name", db::TypeId::VARCHAR, 256);
	auto schema = Schema({c1, c2});
	auto table_name = "user";

	cm->CreateTable(table_name, schema);
	auto &table_meta = cm->GetTableByName(table_name);
	auto table_heap = std::make_unique<TableHeap>(*bpm, table_meta);

	std::vector<RID> rids;
	for (int i = 0; i < 200; ++i) {
		int32_t int_val = i;
		std::string str_val = GenerateRandomString(10, 256);

		auto v1 = Value(db::TypeId::INTEGER, int_val);
		auto v2 = Value(db::TypeId::VARCHAR, str_val);

		auto tuple = Tuple({v1, v2}, schema);
		auto meta = TupleMeta {false};

		auto rid = table_heap->InsertTuple(meta, tuple);
		ASSERT_EQ(rid.has_value(), true);
		rids.push_back(*rid);
		LOG_DEBUG("rid: %d %d", rid->GetPageId().page_number_, rid->GetSlotNum());
	}

	/// flush pages to disk
	bpm->FlushAllPages();
	cm->PersistToDisk();

	auto cm2 = std::make_unique<Catalog>();
	auto dm2 = std::make_unique<DiskManager>(*cm2);
	auto bpm2 = std::make_unique<BufferPool>(buffer_pool_size, *dm2);

	auto &table_meta2 = cm2->GetTableByName(table_name);
	LOG_DEBUG("table_meta2: %s", table_meta2.name_.c_str());

	auto table_heap2 = std::make_unique<TableHeap>(*bpm2, table_meta2);

	for (int i = 0; i < 200; ++i) {
		auto ret = table_heap2->GetTuple(rids[i]);
		if (!ret.has_value()) {
			LOG_ERROR("tuple not found");

			continue;
		}
		auto [meta, tuple] = *ret;
		LOG_DEBUG("tuple: %s", tuple.ToString(schema).c_str());
	}
	auto it = table_heap2->MakeIterator();
	while (!it.IsEnd()) {
		auto [meta, tuple] = *it.GetTuple();
		LOG_DEBUG("tuple: %s", tuple.ToString(schema).c_str());
		++it;
	}
}
} // namespace db
