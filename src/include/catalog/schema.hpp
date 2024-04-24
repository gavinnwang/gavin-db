#pragma once

#include "catalog/column.hpp"
#include <memory>
namespace db {

class Schema;
using SchemaRef = std::shared_ptr<const Schema>;

class Schema {
public:
  explicit Schema(const std::vector<Column> &columns) : columns_(columns) {}

private:
  std::vector<Column> columns_;
};
} // namespace db
