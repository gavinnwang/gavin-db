#pragma once
#include "catalog/schema.hpp"
#include "common/rid.hpp"
namespace db {
class Tuple {

public:
  // Default constructor (to create a dummy tuple)
  Tuple() = default;

private:
  // Get the starting storage address of specific column
  auto GetDataPtr(const Schema &schema, uint32_t column_idx) const -> const
      char *;

  RID rid_{};
  char *data_;
};
} // namespace db
