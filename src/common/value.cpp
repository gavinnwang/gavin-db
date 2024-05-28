#include "common/value.hpp"
#include "common/macros.hpp"
#include "common/type.hpp"
#include <cstdint>
#include <cstring>
#include <variant>
namespace db {
Value::Value(TypeId type_id, const char *data, uint32_t var_len,
             bool manage_data = true) {
  type_id_ = type_id;
  switch (type_id) {
  case TypeId::VARCHAR:
    manage_data_ = manage_data;
    // for now maange data is disabled (cuz haven't test it) TODO
    ASSERT(manage_data_ == true, "Not managing data is not implemented");
    if (manage_data_) {
      char *copy = new char[var_len];
      std::memcpy(copy, data, var_len);
      // copy[var_len] = '\0';
      value_ = copy;
      // value_ = new char[var_len];
      // assert(value_!= nullptr);
      var_len_ = var_len;
      // memcpy(value_.varlen_, data, var_len);
    } else {
      // don't manage our data
      // value_ = data;
      // var_len_ = var_len;
      UNREACHABLE("Not implemented");
    }
    break;
  default:
    UNREACHABLE("Invalid Type for variable-length Value constructor");
  }
}
// copy constructor
Value::Value(const Value &other) {
  type_id_ = other.type_id_;
  var_len_ = other.var_len_;
  manage_data_ = other.manage_data_;
  value_ = other.value_;
  switch (type_id_) {
  case TypeId::VARCHAR:
    if (manage_data_) {
      ASSERT(std::holds_alternative<char *>(value_),
             "Invalid variant type for VARCHAR Value");
      const char *src = std::get<char *>(other.value_);
      char *copy = new char[var_len_];
      memcpy(copy, src, var_len_);
      value_ = copy;
      // value_ = new char[var_len_];
      // memcpy(value_.varlen_, other.value_.varlen_, var_len_);
    } else {
      value_ = other.value_;
    }
    break;
  default:
    value_ = other.value_;
  }
};
// copy assignment operator TODO
auto Value::operator=(Value other) -> Value & {
  if (manage_data_) {
    ASSERT(std::holds_alternative<char *>(value_) &&
               std::holds_alternative<char *>(other.value_),
           "Invalid variant type for VARCHAR Value");
    const char *src = std::get<char *>(other.value_);
    char *copy = new char[var_len_];
    memcpy(copy, src, var_len_);
    value_ = copy;
  } else {
    value_ = other.value_;
  }
  // Swap(*this, other);
  return *this;
}

Value::Value(Value &&other) noexcept : value_(std::move(other.value_)) {
  type_id_ = other.type_id_;
  var_len_ = other.var_len_;
  manage_data_ = other.manage_data_;
  other.type_id_ = TypeId::INVALID;
  other.var_len_ = 0;
  other.manage_data_ = false;
}

// move assignment
auto Value::operator=(Value &&other) noexcept -> Value & {
  if (this != &other) {
    if (std::holds_alternative<char *>(value_)) {
      delete[] std::get<char *>(value_);
    }
    value_ = std::move(other.value_);
  }
  return *this;
};

// deconstructor
Value::~Value() {
  switch (type_id_) {
  case TypeId::VARCHAR:
    if (manage_data_) {
      // delete[] value_;
      delete[] std::get<char *>(value_);
    }
    break;
  default:
    break;
  }
}
void Value::SerializeTo(char *storage) const {
  switch (type_id_) {
  case TypeId::BOOLEAN: {
    ASSERT(std::holds_alternative<int8_t>(value_),
           "Invalid variant type for BOOLEAN Value");
    *reinterpret_cast<int8_t *>(storage) = std::get<int8_t>(value_);
    return;
  }
  case TypeId::INTEGER: {
    *reinterpret_cast<int32_t *>(storage) = std::get<int32_t>(value_);
    return;
  }
  case TypeId::TIMESTAMP: {
    *reinterpret_cast<uint64_t *>(storage) = std::get<uint64_t>(value_);
    return;
  }
  case TypeId::VARCHAR: {
    memcpy(storage, &var_len_, sizeof(uint32_t));
    // memcpy(storage + sizeof(uint32_t), value_.varlen_, var_len_);
    memcpy(storage + sizeof(uint32_t), std::get<char *>(value_), var_len_);
    return;
  }
  case TypeId::INVALID: {
    UNREACHABLE("Cannot serialize invalid type valye");
    return;
  }
  }
}

auto Value::ToString() const -> std::string {
  switch (type_id_) {
  case TypeId::BOOLEAN:
    return std::to_string(std::get<int8_t>(value_));
  case TypeId::INTEGER:
    return std::to_string(std::get<int32_t>(value_));
  case TypeId::TIMESTAMP:
    return std::to_string(std::get<uint64_t>(value_));
  case TypeId::VARCHAR:
    return std::string(std::get<char *>(value_), var_len_);
  case TypeId::INVALID:
    UNREACHABLE("Cannot convert invalid type value to string");
    return "";
  }
}
} // namespace db
