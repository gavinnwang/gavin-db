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

private:
  TypeId type_id_;

  struct timestamp_t {
    int64_t value;
  };

  union Val {
    int32_t integer;
    timestamp_t timestamp;
  };
};
} // namespace db
