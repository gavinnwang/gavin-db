#pragma once

#include "catalog/column.hpp"
#include <memory>
namespace db {

class Schema;
using SchemaRef = std::shared_ptr<const Schema>;

class Schema {
public:
  explicit Schema(const std::vector<Column> &columns);
  auto GetColumns() const -> const std::vector<Column> & { return columns_; }
  auto GetColumn(const uint32_t col_idx) const -> const Column & {
    return columns_[col_idx];
  }
  auto
  TryGetColIdx(const std::string &col_name) const -> std::optional<uint32_t> {
    for (uint32_t i = 0; i < columns_.size(); ++i) {
      if (columns_[i].GetName() == col_name) {
        return std::optional{i};
      }
    }
    return std::nullopt;
  }
  auto GetColumnCount() const -> uint32_t {
    return static_cast<uint32_t>(columns_.size());
  }
  auto GetUnlinedColumns() const -> const std::vector<uint32_t> & {
    return uninlined_columns_;
  }
  auto GetUnlinedColumnCount() const -> uint32_t {
    return static_cast<uint32_t>(uninlined_columns_.size());
  }
  inline auto GetInlinedStorageSize() const -> uint32_t {
    return inline_storage_size_;
  }

private:
  std::vector<Column> columns_;
  std::vector<uint32_t> uninlined_columns_;
  uint32_t inline_storage_size_;
};
} // namespace db
