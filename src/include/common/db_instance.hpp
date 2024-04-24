#pragma once

#include "buffer/buffer_pool_manager.hpp"
#include "storage/disk.hpp"
#include <string>
namespace db {

class DBInstance {
public:
  explicit DBInstance(const std::string &db_file_name);

  auto ExecuteQuery(const std::string &query, std::string* output) -> bool;
  
  ~DBInstance();

private:
  std::unique_ptr<DiskManager> disk_manager_;
  std::unique_ptr<BufferPoolManager> buffer_pool_manager_;
};

}; // namespace db
