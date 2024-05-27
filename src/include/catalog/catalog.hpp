#pragma once

#include "buffer/buffer_pool_manager.hpp"
#include "catalog/schema.hpp"
#include <string>
namespace db {

class Catalog {
public:
  Catalog() = default;
  ~Catalog() = default;

  void Init();

  void CreateTable(const std::string &table_name, const Schema &schema);

private:
  BufferPoolManager *bpm_;
};
} // namespace db
