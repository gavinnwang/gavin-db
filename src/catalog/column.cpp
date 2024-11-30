#include "catalog/column.hpp"

#include "common/type.hpp"

#include <cstdint>
#include <sstream>
namespace db {

auto Column::ToString(bool simplified) const -> std::string {
	std::ostringstream ostream;
	if (simplified) {
		ostream << column_name_ << ":" << Type::TypeIdToString(column_type_);
		if (column_type_ == TypeId::VARCHAR) {
			ostream << "(" << length_ << ")";
		}
		return (ostream.str());
	}

	ostream << "Column[" << column_name_ << ", " << Type::TypeIdToString(column_type_) << ", "
	        << "Offset:" << storage_offset_ << ", ";
	ostream << "Length:" << length_;
	ostream << "]";
	return (ostream.str());
}

} // namespace db
