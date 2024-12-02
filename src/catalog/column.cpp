#include "catalog/column.hpp"

#include "common/type.hpp"

#include <cstdint>
#include <sstream>
namespace db {

auto Column::ToString(bool simplified) const -> std::string {
	if (simplified) {
		if (column_type_ == TypeId::VARCHAR) {
			return fmt::format("{}:{}({})", column_name_, Type::TypeIdToString(column_type_), length_);
		}
		return fmt::format("{}:{}", column_name_, Type::TypeIdToString(column_type_));
	}

	return fmt::format("Column[{}, {}, Offset:{}, Length:{}]", column_name_, Type::TypeIdToString(column_type_),
	                   storage_offset_, length_);
}

} // namespace db
