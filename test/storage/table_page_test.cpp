#include "buffer/buffer_pool_manager.hpp"
#include "common/debug.hpp"
#include "storage/table_heap.hpp"
#include "storage/tuple.hpp"
#include "gtest/gtest.h"
#include <iostream>

TEST(StorageTest, TablePageTest) {
  const size_t buffer_pool_size = 10;
  auto *dm = new db::DiskManager("test.db");
  auto *bpm = new db::BufferPoolManager(buffer_pool_size, dm, 0);

  auto table_heap = new db::TableHeap(bpm);
  auto c1 = db::Column("user_id", db::TypeId::INTEGER);
  auto c2 = db::Column("user_name", db::TypeId::VARCHAR, 256);
  auto schema = db::Schema({c1, c2});
  auto v1 = db::Value(db::TypeId::INTEGER, 239);
  auto v2 = db::Value(db::TypeId::VARCHAR, "hihihi", 6, true);

  auto tuple = db::Tuple({std::move(v1), std::move(v2)}, schema);
  // std::cout << tuple.ToString(schema) << std::endl;

  auto meta = db::TupleMeta{false};
  auto rid = table_heap->InsertTuple(meta, tuple);
  auto [meta2, tuple2] = table_heap->GetTuple(*rid);
  // std::cout << tuple2.ToString(schema) << std::endl;

  // assett that the tuple is the same as the inserted tuple
  ASSERT_EQ(tuple.ToString(schema), tuple2.ToString(schema));
}
