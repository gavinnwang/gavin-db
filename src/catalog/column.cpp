#include "catalog/column.hpp"
#include <sstream>
namespace db {

auto Column::ToString(bool simplified) const -> std::string {
  if (simplified) {
    std::ostringstream os;
    os << column_name_ << ":" << Type::TypeIdToString(column_type_);
    if (column_type_ == VARCHAR) {
      os << "(" << length_ << ")";
    }
    return (os.str());
  }

  std::ostringstream os;

  os << "Column[" << column_name_ << ", " << Type::TypeIdToString(column_type_)
     << ", " << "Offset:" << column_offset_ << ", ";
  os << "Length:" << length_;
  os << "]";
  return (os.str());
}

} // namespace db
