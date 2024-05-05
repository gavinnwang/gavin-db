#include "buffer/buffer_pool_manager.hpp"
#include "gtest/gtest.h"

TEST(StorageTest, TablePageTest) {
  const size_t buffer_pool_size = 10;
  auto *dm = new db::DiskManager("test.db");
  auto *bpm = new db::BufferPoolManager(buffer_pool_size, dm, 0);
  
  
}
