#include "catalog/catalog_manager.hpp"

#include "common/typedef.hpp"
#include "storage/file_path_manager.hpp"

namespace db {

std::optional<table_oid_t> CatalogManager::CreateTable(const std::string &table_name, const Schema &schema) {
	if (table_names_.contains(table_name)) {
		return std::nullopt;
	}
	table_oid_t table_oid = tables_.size();
	table_names_.emplace(table_name, table_oid);

	auto table_meta = std::make_unique<TableMeta>(schema, table_name, table_oid);

	tables_.insert({table_oid, std::move(table_meta)});

	CreateFileIfNotExists(FilePathManager::GetInstance().GetTableMetaPath(table_name));
	CreateFileIfNotExists(FilePathManager::GetInstance().GetTableDataPath(table_name));
	PersistToDisk();
	// create table data and meta files
	return table_oid;
}

} // namespace db
