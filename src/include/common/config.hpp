#pragma once

#include <cstddef>
#include <string>
namespace db {
static constexpr int INVALID_PAGE_ID = -1; // invalid page id
static constexpr int INVALID_TABLE_OID = -1;
static constexpr int PAGE_SIZE = 4096; // size of a data page in byte
const std::string DEFAULT_DB_NAME = "gavindb";
static constexpr int DEFAULT_POOL_SIZE = 10;

static constexpr int INDEX_KEY_SIZE = 8;
static constexpr int VARCHAR_DEFAULT_LENGTH = 128; // default length for varchar when constructing the column

// TODO(gavinwang): make system catalog table when ready
static constexpr int SYSTEM_CATALOG_ID = -1;
} // namespace db
