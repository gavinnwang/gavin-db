#include "query/binder/statement/insert_statement.hpp"

namespace db {
std::string InsertStatement::ToString() const {
	return fmt::format("BoundInsert {{\n  table={},\n  select={} }}", table_->ToString(), select_->ToString());
}
} // namespace db
