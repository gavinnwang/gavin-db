#pragma once

#include "buffer/buffer_pool_manager.hpp"
#include "common/config.hpp"
#include <cstdint>
namespace db {

static constexpr uint64_t TABLE_INFO_PAGE_HEADER_SIZE = 16;

// need to store table for tables, indexes, also next page id
class CatalogPage {
public:
  void Init(BufferPoolManager *bpm);

private:
  char page_start_[0];
  const uint32_t MAGIC_NUMBER{0};
  page_id_t next_page_id_{INVALID_PAGE_ID};
  page_id_t tables_table_page_id_{INVALID_PAGE_ID};
  page_id_t indexes_table_page_id_{INVALID_PAGE_ID};
};

static_assert(sizeof(CatalogPage) == TABLE_INFO_PAGE_HEADER_SIZE);
} // namespace db
