#include "binder/statement/select_statement.hpp"
namespace db {
std::string SelectStatement::ToString() const {
	return fmt::format("BoundSelect {{\n  table={},\n  select_list={}", table_->ToString(), select_list_);
}
} // namespace db
