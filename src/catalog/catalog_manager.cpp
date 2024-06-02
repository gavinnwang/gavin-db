#include "catalog/catalog_manager.hpp"

#include "common/typedef.hpp"
#include "storage/file_path_manager.hpp"

namespace db {

table_oid_t CatalogManager::CreateTable(const std::string &table_name, const Schema &schema) {
	if (table_names_.contains(table_name)) {
		throw Exception("Table already exists");
	}
	table_oid_t table_oid = tables_.size();
	table_names_.emplace(table_name, table_oid);
	std::cout << "table_oid: " << table_oid << std::endl;

	tables_.emplace(table_oid, std::make_shared<TableInfo>(schema, table_name, table_oid));

	PersistToDisk();

	// create folders for table
	CreateFileIfNotExists(FilePathManager::GetInstance().GetTableMetaPath(table_name));
	CreateFileIfNotExists(FilePathManager::GetInstance().GetTableDataPath(table_name));
	return table_oid;
}
} // namespace db
