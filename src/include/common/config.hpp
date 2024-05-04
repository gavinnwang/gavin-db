#pragma once
#include <cstddef>
#include <cstdint>
namespace db {
static constexpr int INVALID_PAGE_ID = -1; // invalid page id
static constexpr int PAGE_SIZE = 4096;     // size of a data page in byte

using frame_id_t = int32_t;   // frame id type
using page_id_t = int32_t;    // page id type
using slot_offset_t = size_t; // slot offset type
using table_oid_t = uint32_t;
} // namespace db
