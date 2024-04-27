#pragma once

#include "common/type.hpp"
#include <cstdint>
namespace db {
class Value {
public:
  // create an INTEGER value
  Value(int32_t val);
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

  inline auto GetStorageSize() const -> uint32_t {
    return Type::TypeSize(type_id_, var_len_);
  }

  auto GetData(const Value &val) const -> const char *;
  auto GetStorageSize(const Value &val) const -> uint32_t;
  void SerializeTo(const Value &val, char *storage) const;
  auto DeserializeFrom(const char *storage) const -> Value;

private:
  TypeId type_id_;

  struct timestamp_t {
    int64_t value;
  };

  uint32_t var_len_;

  union Val {
    int32_t integer;
    timestamp_t timestamp;
  };
};
} // namespace db
