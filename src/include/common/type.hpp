#pragma once

#include "common/macros.hpp"
#include <string>
namespace db {
enum TypeId : uint8_t { INVALID = 0, BOOLEAN, INTEGER, TIMESTAMP, VARCHAR };

class Type {
public:
  static auto TypeSize(TypeId type_id, uint32_t length = 0) -> uint8_t {
    switch (type_id) {
    case TypeId::BOOLEAN:
      return 1;
    case TypeId::INTEGER:
      return 4;
    case TypeId::TIMESTAMP:
      return 8;
    case TypeId::VARCHAR:
      return length;
    default: {
      UNREACHABLE("Cannot get size of invalid type");
    }
    }
  }
  static auto TypeIdToString(TypeId type_id) -> std::string {
    switch (type_id) {
    case TypeId::BOOLEAN:
      return "BOOLEAN";
    case TypeId::INTEGER:
      return "INTEGER";
    case TypeId::TIMESTAMP:
      return "TIMESTAMP";
    case TypeId::VARCHAR:
      return "VARCHAR";
    default: {
      UNREACHABLE("Unkown type");
    }
    }
  }
};
} // namespace db
