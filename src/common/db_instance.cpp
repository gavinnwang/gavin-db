#include "common/db_instance.hpp"
#include "catalog/column.hpp"
#include "catalog/schema.hpp"
#include "storage/disk.hpp"
#include <iostream>
#include <memory>

namespace db {

DBInstance::DBInstance(const std::string &db_file_name) {
  std::cout << db_file_name << std::endl;
  auto col= Column("c1", TypeId::INTEGER);
  auto s = Schema({col});
  char buf[PAGE_SIZE] = {0};
  char data[PAGE_SIZE] = {0};
  std::string db_file("test.db");
  std::strncpy(data, "A test string. ha ha ha ha.", sizeof(data));
  disk_manager_ = std::make_unique<DiskManager>("temp.db");
  disk_manager_->WritePage(0, data);
  disk_manager_->ReadPage(0, buf);
  std::cout << buf;
  disk_manager_->ShutDown();
};
}
