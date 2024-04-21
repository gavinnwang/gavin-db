#pragma once

#include <string>
namespace db {

class DBInstance {
public:
  explicit DBInstance(const std::string &db_file_name);
};

}; // namespace db
