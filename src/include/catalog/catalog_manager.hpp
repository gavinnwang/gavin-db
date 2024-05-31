#pragma once

#include "buffer/buffer_pool_manager.hpp"
#include "catalog/schema.hpp"
#include "storage/table_info.hpp"
#include <memory>
#include <string>
#include <unordered_map>
namespace db {

class CatalogManager {
public:
  CatalogManager() = default;
  ~CatalogManager() = default;

  void Init();

  void CreateTable(const std::string &table_name, const Schema &schema);

  void GeTableInfo(const std::string &table_name) const;

private:
  std::unordered_map<std::string, std::shared_ptr<TableInfo>> tables_;
  std::weak_ptr<BufferPoolManager> bpm_;
};
} // namespace db
