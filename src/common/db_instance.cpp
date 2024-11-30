#include "common/db_instance.hpp"

#include "catalog/schema.hpp"
namespace db {

void DB::HandleCreateStatement([[maybe_unused]] Transaction &txn, const CreateStatement &stmt) {
	std::unique_lock<std::shared_mutex> l(catalog_manager_lock_);
	catalog_manager_->CreateTable(stmt.table_name_, Schema {stmt.columns_});

	if (!stmt.primary_key_.empty()) {
	}
}

void DB::SetUpInternalSystemCatalogTable() {
	// check if it already exist
	auto sys_catalog_path = FilePathManager::GetInstance().GetSystemCatalogPath();

	if (fs::exists(sys_catalog_path)) {
		// if already exist
	} else {
		// need to set up
		CreateFileIfNotExists(sys_catalog_path);
	}
}
} // namespace db
