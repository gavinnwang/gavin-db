#include "buffer/buffer_pool_manager.hpp"
#include "common/config.hpp"
#include "storage/table_heap.hpp"
#include "storage/table_info_page.hpp"
#include "storage/tuple.hpp"
#include "gtest/gtest.h"

TEST(StorageTest, SimpleTableInfoPageTest) {
  const size_t buffer_pool_size = 10;
  auto *dm = new db::DiskManager("test.db");
  auto *bpm = new db::BufferPoolManager(buffer_pool_size, dm, 0);

  db::page_id_t page_id;
  auto pg = bpm->NewPageGuarded(&page_id);
  auto pgw = pg.UpgradeWrite();
  auto table_info_page = pgw.AsMut<db::TableInfoPage>();
  auto c1 = db::Column("user_id", db::TypeId::INTEGER);
  auto c2 = db::Column("user_name", db::TypeId::VARCHAR, 256);
  auto schema = db::Schema({c1, c2});
  auto table_name = "usr";
  table_info_page->Init(table_name, schema, 0);
  auto table_info = table_info_page->GetTableInfo();
  ASSERT_EQ(table_info.name_, table_name);
  ASSERT_EQ("Schema(user_id:INTEGER, user_name:VARCHAR(256))",
            table_info.schema_.ToString());
  ASSERT_EQ(0, table_info.table_oid_);
}
