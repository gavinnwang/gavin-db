#include "common/value.hpp"
#include "common/macros.hpp"
#include "common/type.hpp"
#include <cstdint>
namespace db {
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
