#pragma once
#include "storage/table_page.hpp"
#include <cstdint>

namespace db {

struct TableInfo {
  Schema schema_;
  const std::string name_;
  const table_oid_t table_oid_;

  TableInfo(Schema schema, std::string name, table_oid_t table_oid)
      : schema_{std::move(schema)}, name_{std::move(name)},
        table_oid_{table_oid} {}

  void SerializeTo(char *storage) const {
    uint32_t offset = 0;
    schema_.SerializeTo(storage + offset);
    offset += schema_.GetSerializationSize();
    // now serialize the table name length and string
    uint32_t table_name_size = name_.size();
    memcpy(storage + offset, &table_name_size, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(storage + offset, name_.data(), table_name_size);
    offset += table_name_size;
    memcpy(storage + offset, &table_oid_, sizeof(uint32_t));
  }
};

static constexpr uint64_t TABLE_INFO_PAGE_HEADER_SIZE = 8;

class TableInfoPage {
  // stores the schema and table name info to disk
  friend class TablePage;

public:
  void Init(const std::string name, const Schema &schema,
            const table_oid_t table_oid);
  auto GetTableInfo() -> TableInfo;
  void UpdateTableSchema(const Schema &schema);

private:
  void StoreTableInfo(const TableInfo &table_info);
  char page_start_[0];
  page_id_t first_table_page_id_;
  uint32_t table_info_offset_{0};
};

static_assert(sizeof(TableInfoPage) == TABLE_INFO_PAGE_HEADER_SIZE);
} // namespace db
