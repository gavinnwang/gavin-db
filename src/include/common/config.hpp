#pragma once

#include "common/typedef.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
namespace db {
static constexpr page_id_t INVALID_PAGE_ID = -1; // invalid page id
static constexpr table_oid_t INVALID_TABLE_OID = -1;
static constexpr uint32_t PAGE_SIZE = 4096; // size of a data page in byte
const std::string DEFAULT_DB_NAME = "gavindb";
static constexpr uint32_t DEFAULT_POOL_SIZE = 10;
static constexpr uint32_t INDEX_KEY_SIZE = 8;
static constexpr uint32_t VARCHAR_DEFAULT_LENGTH = 128; // default length for varchar when constructing the column
static constexpr table_oid_t SYSTEM_CATALOG_ID = -1;
static constexpr timestamp_t INVALID_TS = -1;
const txn_id_t TXN_START_ID = 1LL << 62; // first txn id
} // namespace db
