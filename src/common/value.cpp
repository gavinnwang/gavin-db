#include "common/value.hpp"
#include "common/macros.hpp"
#include "common/type.hpp"
#include <cstdint>
namespace db {
Value::Value(TypeId type_id, const char *data, uint32_t var_len,
             bool manage_data) {
  switch (type_id) {
  case TypeId::VARCHAR:
    manage_data_ = manage_data;
    if (manage_data_) {
      assert(var_len < 50);
      value_.varlen_ = new char[var_len];
      assert(value_.varlen_ != nullptr);
      var_len_ = var_len;
      memcpy(value_.varlen_, data, var_len);
    } else {
      // don't manage our data
      value_.const_varlen_ = data;
      var_len_ = var_len;
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
      value_.varlen_ = new char[var_len_];
      memcpy(value_.varlen_, other.value_.varlen_, var_len_);
    } else {
      value_ = other.value_;
    }
    break;
  default:
    value_ = other.value_;
  }
};
// copy assignment operator
auto Value::operator=(Value other) -> Value & {
  Swap(*this, other);
  return *this;
}
// deconstructor
Value::~Value() {
  switch (type_id_) {
  case TypeId::VARCHAR:
    if (manage_data_) {
      delete[] value_.varlen_;
    }
    break;
  default:
    break;
  }
}
void Value::SerializeTo(char *storage) const {
  switch (type_id_) {
  case TypeId::BOOLEAN: {
    *reinterpret_cast<int8_t *>(storage) = value_.boolean_;
    return;
  }
  case TypeId::INTEGER: {
    *reinterpret_cast<int32_t *>(storage) = value_.integer_;
    return;
  }
  case TypeId::TIMESTAMP: {
    *reinterpret_cast<uint32_t *>(storage) = value_.integer_;
    return;
  }
  case TypeId::VARCHAR: {
    memcpy(storage, &var_len_, sizeof(uint32_t));
    memcpy(storage + sizeof(uint32_t), value_.varlen_, var_len_);
    return;
  }
  case TypeId::INVALID: {
    UNREACHABLE("Cannot serialize invalid type valye");
    return;
  }
  }
}
} // namespace db
