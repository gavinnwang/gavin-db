#pragma once

#include "storage/disk.hpp"
#include <string>
namespace db {

class DBInstance {
public:
  explicit DBInstance(const std::string &db_file_name);
private:
  std::unique_ptr<DiskManager> disk_manager_;
};

}; // namespace db
