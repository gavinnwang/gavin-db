#pragma once
#include "common/typedef.hpp"

#include <cstddef>
#include <string>
namespace db {
static constexpr int INVALID_PAGE_ID = -1; // invalid page id
static constexpr int INVALID_TABLE_OID = -1;
static constexpr int PAGE_SIZE = 4096; // size of a data page in byte
// static constexpr uint32_t MAGIC_NUMBER = 0x20050313;
const std::string DEFAULT_DB_NAME = "gavindb";
} // namespace db
