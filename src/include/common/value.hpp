#pragma once

#include "common/config.hpp"
#include "common/macros.hpp"
#include "common/type.hpp"
#include <cstdint>
#include <variant>
namespace db {
class Value {
public:
  // create an value with type_id
  Value(TypeId type) : type_id_(type) {}

  // create an INVALID value
  Value() : Value(TypeId::INVALID) {}

  template <typename T>
  Value(TypeId type, T &&i) : type_id_(type), value_(std::forward<T>(i)){};

  // Get the length of the variable length data
  inline auto GetStorageSize() const -> uint32_t {
    return Type::TypeSize(type_id_, GetVarlenStorageSize());
  }

  void SerializeTo(char *storage) const;

  auto ToString() const -> std::string;

  static auto DeserializeFrom(const char *storage,
                              const TypeId type_id) -> Value {
    switch (type_id) {
    case TypeId::BOOLEAN: {

      int8_t val = *reinterpret_cast<const int8_t *>(storage);
      return {type_id, val};
    }
    case TypeId::INTEGER: {
      int32_t val = *reinterpret_cast<const int32_t *>(storage);
      return {type_id, val};
    }
    case TypeId::TIMESTAMP: {
      uint64_t val = *reinterpret_cast<const uint32_t *>(storage);
      return {type_id, val};
    }
    case TypeId::VARCHAR: {
      uint32_t var_len = *reinterpret_cast<const uint32_t *>(storage);

      ASSERT(var_len < PAGE_SIZE, "Invalid varchar length");
      return {type_id, std::string(storage + sizeof(uint32_t), var_len)};
    }
    case TypeId::INVALID: {
      UNREACHABLE("Value has invalid type id");
    }
    }
  }

private:
  auto GetVarlenStorageSize() const -> uint32_t {
    if (type_id_ != TypeId::VARCHAR) {
      return 0;
    } else {
      return std::get<std::string>(value_).size();
    }
  }
  TypeId type_id_;

  // Define the variant to hold all possible types
  using Val = std::variant<int8_t, int32_t, uint64_t, std::string>;

  Val value_;
};
} // namespace db
