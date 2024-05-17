#include "buffer/buffer_pool_manager.hpp"
#include "storage/table_heap.hpp"
#include "storage/table_info_page.hpp"
#include "storage/tuple.hpp"
#include "gtest/gtest.h"
#include <cstdint>

TEST(StorageTest, SimpleTableHeapTest) {
  const size_t buffer_pool_size = 10;
  auto *dm = new db::DiskManager("test.db");
  auto *bpm = new db::BufferPoolManager(buffer_pool_size, dm, 0);

  db::page_id_t table_info_page_id;
  auto guard = bpm->NewPageGuarded(&table_info_page_id);
  ASSERT_NE(table_info_page_id, db::INVALID_PAGE_ID);

  auto c1 = db::Column("user_id", db::TypeId::INTEGER);
  auto c2 = db::Column("user_name", db::TypeId::VARCHAR, 256);
  auto schema = db::Schema({c1, c2});
  auto table_name = "usr";

  auto table_info_wpg = guard.UpgradeWrite();
  auto table_info_page = table_info_wpg.AsMut<db::TableInfoPage>();
  table_info_page->Init(table_name, schema, 0);
  table_info_wpg.Drop();

  auto table_heap = new db::TableHeap(bpm, table_info_page_id);
  int32_t int_val = 2392;
  std::string str_val = "hhihi";
  auto v1 = db::Value(db::TypeId::INTEGER, int_val);
  auto v2 = db::Value(db::TypeId::VARCHAR, str_val.data(),
                      static_cast<uint32_t>(str_val.size()), true);

  auto tuple = db::Tuple({v1, v2}, schema);

  auto meta = db::TupleMeta{false};
  auto rid1 = table_heap->InsertTuple(meta, tuple);
  auto rid2 = table_heap->InsertTuple(meta, tuple);
  auto rid3 = table_heap->InsertTuple(meta, tuple);

  ASSERT_EQ(rid1.has_value(), true);
  ASSERT_EQ(rid2.has_value(), true);
  ASSERT_EQ(rid3.has_value(), true);

  auto [meta1, tuple1] = table_heap->GetTuple(*rid1);
  auto [meta2, tuple2] = table_heap->GetTuple(*rid2);

  ASSERT_EQ(tuple.ToString(schema),
            "(" + std::to_string(int_val) + ", " + str_val + ")");
  ASSERT_EQ(tuple.ToString(schema), tuple1.ToString(schema));
  ASSERT_EQ(tuple.ToString(schema), tuple2.ToString(schema));
}
