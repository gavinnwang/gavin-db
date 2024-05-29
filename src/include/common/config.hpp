#pragma once
#include "common/typedef.hpp"

#include <cstddef>
#include <cstdint>
namespace db {
static constexpr int INVALID_PAGE_ID = -1; // invalid page id
static constexpr int PAGE_SIZE = 4096;     // size of a data page in byte
static constexpr uint32_t MAGIC_NUMBER = 0x20050313;

static constexpr page_id_t CATALOG_PAGE_ID = 0; // fixed page id of the catalog page
} // namespace db
