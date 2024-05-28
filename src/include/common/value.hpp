#pragma once

#include "common/config.hpp"
#include "common/macros.hpp"
#include "common/type.hpp"
#include <cstdint>
#include <string_view>
namespace db {
class Value {
public:
  // create an value with type_id
  Value(TypeId type)
      : type_id_(type), var_len_(INVALID_VAR_LEN), manage_data_(false) {}

  // create an INVALID value
  Value() : Value(TypeId::INVALID) {}

  // create an BOOLEAN value
  Value(TypeId type, int8_t i) : Value(type) { value_ = i; }
  // INTEGER
  Value(TypeId type, int32_t i) : Value(type) { value_ = i; }
  // TIMESTAMP
  Value(TypeId type, uint64_t i) : Value(type) { value_ = i; }
  // VARCHAR
  Value(TypeId type_id, const char *data, uint32_t len, bool manage_data);
  // copy constructor
  Value(const Value &other);
  // deconstructor
  ~Value();
  // copy assignment
  auto operator=(Value other) -> Value &;
  // move constructor
  Value(Value &&other) noexcept;
  // move assignment
  auto operator=(Value &&other) noexcept -> Value &;

  // friend void Swap(Value &first, Value &second) {
  //   std::swap(first.value_, second.value_);
  //   std::swap(first.var_len_, second.var_len_);
  //   std::swap(first.manage_data_, second.manage_data_);
  //   std::swap(first.type_id_, second.type_id_);
  // }

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

  bool manage_data_;

  // Define the variant to hold all possible types
  using Val = std::variant<int8_t, int32_t, uint64_t, char *, std::string_view>;

  // Store the variant
  Val value_;
  // union Val {
  //   int8_t boolean_;
  //   int32_t integer_;
  //   uint64_t timestamp_;
  //   char *varlen_;
  //   const char *const_varlen_;
  // } value_;

  // TODO look into variant
};
} // namespace db
