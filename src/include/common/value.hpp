#pragma once

#include "common/macros.hpp"
#include "common/type.hpp"
#include <cstdint>
namespace db {
class Value {
public:
  Value() : type_id_(TypeId::INVALID), var_len_(INVALID_VAR_LEN) {};

    // create an BOOLEAN value
    Value(TypeId type, int8_t i) : type_id_(type), var_len_(INVALID_VAR_LEN), value_{.boolean_ = i} {}
    
    // INTEGER
    Value(TypeId type, int32_t i) : type_id_(type), var_len_(INVALID_VAR_LEN), value_{.integer_ = i} {}
    
    // TIMESTAMP
    Value(TypeId type, uint64_t i) : type_id_(type), var_len_(INVALID_VAR_LEN), value_{.timestamp_ = i} {}
  // VARCHAR
  Value(TypeId type, const char *data, uint32_t len, bool manage_data);
  // copy constructor
  Value(const Value &other);
  // move constructor
  Value(Value &&other) noexcept;
  // deconstructor
  ~Value();
  // copy assignment
  Value &operator=(const Value &other);
  // move assignment
  Value &operator=(Value &&other) noexcept;

  // Get the length of the variable length data
  inline auto GetStorageSize() const -> uint32_t {
    return Type::TypeSize(type_id_, var_len_);
  }

  auto GetVarlenData(const Value &val) const -> const char *;

  inline auto GetVarlenStorageSize() const -> uint32_t {
    UNREACHABLE("Incompatible value type to call GetVarlenStorageSize");
    return var_len_;
  }

  void SerializeTo(char *storage) const;

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
      return {type_id, storage + sizeof(uint32_t), var_len, true};
    }
    case TypeId::INVALID: {
        UNREACHABLE("Value has invalid type id");
    }
    }
  }

private:
  static constexpr uint32_t INVALID_VAR_LEN = 0;
  TypeId type_id_;

  uint32_t var_len_;

  union Val {
    int8_t boolean_;
    int32_t integer_;
    uint64_t timestamp_;
    char* varlen_;
    const char *const_varlen_;
  } value_;
};
} // namespace db
