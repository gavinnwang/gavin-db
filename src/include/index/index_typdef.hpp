#pragma once
#include "common/rid.hpp"

#include <array>
namespace db {

using IndexKeyType = std::array<data_t, INDEX_KEY_SIZE>;
using IndexValueType = RID;
using InternalValueType = page_id_t;

} // namespace db
