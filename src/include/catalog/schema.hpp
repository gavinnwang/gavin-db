#pragma once

#include "catalog/column.hpp"
#include <memory>
namespace db {

class Schema;
using SchemaRef = std::shared_ptr<const Schema>;

class Schema {
public:
  explicit Schema(const std::vector<Column> &columns);

private:
  std::vector<Column> columns_;
  std::vector<uint32_t> uninlined_columns_;
};
} // namespace db
