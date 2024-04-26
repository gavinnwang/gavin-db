#pragma once

#include "common/macros.hpp"
#include "common/type.hpp"
#include <string>

namespace db {
class Column {
  friend class Schema;

public:
  // fixed length col
  Column(std::string column_name, TypeId type)
      : column_name_(std::move(column_name)), column_type_(type),
        length_(Type::TypeSize(type)) {
    ASSERT(type != TypeId::VARCHAR, "Wrong constructor for VARCHAR type.");
  }

  // variable length col
  Column(std::string column_name, TypeId type, uint32_t length)
      : column_name_(std::move(column_name)), column_type_(type),
        length_(Type::TypeSize(type, length)) {
    ASSERT(type == TypeId::VARCHAR, "Wrong constructor for non-VARCHAR type.");
  }

  auto GetName() const -> std::string { return column_name_; }
  auto GetStorageSize() const -> uint32_t { return length_; }
  auto GetOffset() const -> uint32_t { return column_offset_; }
  auto GetType() const -> TypeId { return column_type_; }
  auto IsInlined() const -> bool { return column_type_ != TypeId::VARCHAR; }

  auto ToString(bool simplified = true) const -> std::string;

private:
  std::string column_name_;
  TypeId column_type_;
  // size of col
  uint32_t length_;
  // offset in the tuple
  uint32_t column_offset_ = 0;
};
} // namespace db
