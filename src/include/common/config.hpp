#pragma once

#include <cstddef>
#include <string>
namespace db {
static constexpr int INVALID_PAGE_ID = -1; // invalid page id
static constexpr int INVALID_TABLE_OID = -1;
static constexpr int PAGE_SIZE = 4096; // size of a data page in byte
const std::string DEFAULT_DB_NAME = "gavindb";

static constexpr int INDEX_KEY_SIZE = 8;

// TODO make system catalog table when ready
static constexpr int SYSTEM_CATALOG_ID = -1;
} // namespace db
