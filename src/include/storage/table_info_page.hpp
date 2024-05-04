#pragma once
#include "storage/table_page.hpp"
#include <cstdint>
namespace db {

struct TableInfo {
  Schema schema_;
  const std::string name_;
  const table_oid_t table_oid_;
};

class TableInfoPage {
  friend class TablePage;

public:
  void Init();
  auto GetTableInfo() -> std::optional<TableInfo>;
  void UpdateTableSchema(const Schema &schema);
  

private:
  char page_start_[0];
  page_id_t first_table_page_id_;
  uint16_t num_pages_; 
  uint16_t storage_size_;
};
} // namespace db
