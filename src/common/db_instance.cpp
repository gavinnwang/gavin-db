#include "common/db_instance.hpp"

#include "catalog/schema.hpp"
namespace db {

void DB::HandleCreateStatement([[maybe_unused]] Transaction &txn, const CreateStatement &stmt) {
	std::unique_lock<std::shared_mutex> l(catalog_manager_lock_);
	catalog_manager_->CreateTable(stmt.table_name_, Schema {stmt.columns_});
}
} // namespace db
