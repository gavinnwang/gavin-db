#pragma once

#include "common/config.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
namespace db {
class RID;

using idx_t = uint64_t;
using frame_id_t = int32_t; // frame id type
using page_id_t = int32_t;  // page id type
using column_t = uint32_t;
using table_oid_t = int32_t;
using index_oid_t = int32_t;
using slot_offset_t = idx_t; // slot offset type
using data_t = uint8_t;
using data_ptr_t = data_t *;
using const_data_ptr_t = const data_t *;

using IndexKeyType = std::array<data_t, INDEX_KEY_SIZE>;
using IndexValueType = RID;
using InternalValueType = page_id_t;

template <class SRC>
data_ptr_t data_ptr_cast(SRC *src) { // NOLINT: naming
	return reinterpret_cast<data_ptr_t>(src);
}

template <class SRC>
const_data_ptr_t const_data_ptr_cast(const SRC *src) { // NOLINT: naming
	return reinterpret_cast<const_data_ptr_t>(src);
}

template <class SRC>
const char *const_char_ptr_cast(const SRC *src) { // NOLINT: naming
	return reinterpret_cast<const char *>(src);
}

} // namespace db
