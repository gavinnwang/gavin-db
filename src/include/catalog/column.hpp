#pragma once

#include <string>

#include "common/macros.hpp"
#include "common/type.hpp"

namespace db {
class Column {
  friend class Schema;

public:
  // Non-variable-length varchar
  Column(std::string column_name, TypeId type)
      : column_name_(std::move(column_name)), column_type_(type),
        fixed_length_(TypeSize(type)) {
    ASSERT(type != TypeId::VARCHAR, "Wrong constructor for VARCHAR type.");
  }

  // Variable-length non varchar col
  Column(std::string column_name, TypeId type, uint32_t length)
      : column_name_(std::move(column_name)), column_type_(type),
        fixed_length_(TypeSize(type)), variable_length_(length) {
    ASSERT(type == TypeId::VARCHAR, "Wrong constructor for non-VARCHAR type.");
  }

private:
  static auto TypeSize(TypeId type) -> uint8_t {
    switch (type) {
    case TypeId::BOOLEAN:
      return 1;
      return 2;
    case TypeId::INTEGER:
      return 4;
    case TypeId::TIMESTAMP:
    case TypeId::VARCHAR:
      return 8;
    default: {
      UNREACHABLE("Cannot get size of invalid type");
    }
    }
  }
  std::string column_name_;
  /** Column value's type. */
  TypeId column_type_;
  /** For a non-inlined column, this is the size of a pointer. Otherwise, the
   * size of the fixed length column. */
  uint32_t fixed_length_;
  /** For an inlined column, 0. Otherwise, the length of the variable length
   * column. */
  uint32_t variable_length_{0};
  /** Column offset in the tuple. */
  uint32_t column_offset_{0};
};
} // namespace db
