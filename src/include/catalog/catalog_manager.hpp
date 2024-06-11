#pragma once

#include "catalog/schema.hpp"
#include "common/fs_utils.hpp"
#include "common/typedef.hpp"
#include "storage/disk_manager.hpp"
#include "storage/file_path_manager.hpp"
#include "storage/serializer/binary_deserializer.hpp"
#include "storage/serializer/binary_serializer.hpp"
#include "storage/serializer/file_stream.hpp"
#include "storage/serializer/serialization_traits.hpp"
#include "storage/table/table_meta.hpp"

#include <memory>
#include <string>
#include <unordered_map>
namespace db {
class CatalogManager {
public:
	CatalogManager() {
		std::filesystem::path catalog_path = FilePathManager::GetInstance().GetSystemCatalogPath();
		CreateFileIfNotExists(catalog_path);
		if (GetFileSize(catalog_path) > 0) {
			auto catalog_fs = FileStream(catalog_path);
			BinaryDeserializer deserializer(catalog_fs);
			Deserialize(deserializer);
		}
		EnsureTableFilesExist();
	}

	table_oid_t TryCreateTable(const std::string &table_name, const Schema &schema);

	table_oid_t CreateTable(const std::string &table_name, const Schema &schema);

	std::shared_ptr<TableMeta> GetTable(const std::string &table_name) const {
		if (table_names_.find(table_name) == table_names_.end()) {
			throw Exception("Table not found when getting table info");
		}
		return tables_.at(table_names_.at(table_name));
	};

	std::string &GetTableName(const table_oid_t table_oid) const {
		if (tables_.find(table_oid) == tables_.end()) {
			throw Exception("Table not found when getting table name");
		}
		return tables_.at(table_oid)->name_;
	}
	// Get the last page id of the table
	page_id_t GetLastPageId(const table_oid_t table_oid) const {
		if (tables_.find(table_oid) == tables_.end()) {
			throw Exception("Table not found when getting last page id");
		}
		return tables_.at(table_oid)->last_table_page_id_;
	}

	void PersistToDisk() {
		// persist the catalog to disk
		std::filesystem::path catalog_path = FilePathManager::GetInstance().GetSystemCatalogPath();
		// CreateFileIfNotExists(catalog_path)
		auto catalog_fs = FileStream(catalog_path);
		BinarySerializer serializer(catalog_fs);
		Serialize(serializer);

		// persist all table meta to disk
		for (const auto &[table_oid, table_meta] : tables_) {
			std::filesystem::path table_meta_path = FilePathManager::GetInstance().GetTableMetaPath(table_meta->name_);
			// CreateFileIfNotExists(table_meta_path);
			auto table_meta_fs = FileStream(table_meta_path);
			BinarySerializer table_meta_serializer(table_meta_fs);
			table_meta->Serialize(table_meta_serializer);
		}
	}

	void Serialize(Serializer &serializer) const {
		serializer.WritePropertyWithDefault(100, "tables", tables_,
		                                    std::unordered_map<table_oid_t, std::shared_ptr<TableMeta>>());
		serializer.WritePropertyWithDefault(101, "table_names", table_names_,
		                                    std::unordered_map<std::string, table_oid_t>());
	}

	void Deserialize(Deserializer &deserializer) {
		deserializer.ReadPropertyWithDefault(100, "tables", tables_,
		                                     std::unordered_map<table_oid_t, std::shared_ptr<TableMeta>>());
		deserializer.ReadPropertyWithDefault(101, "table_names", table_names_,
		                                     std::unordered_map<std::string, table_oid_t>());
	}

private:
	void EnsureTableFilesExist() {
		for (const auto &[table_name, table_oid] : table_names_) {
			auto data_exists = std::filesystem::exists(FilePathManager::GetInstance().GetTableDataPath(table_name));
			auto meta_exists = std::filesystem::exists(FilePathManager::GetInstance().GetTableMetaPath(table_name));
			if (!data_exists || !meta_exists) {
				throw Exception("Data file or meta file not found for table " + table_name);
			}
		}
	};
	std::unordered_map<table_oid_t, std::shared_ptr<TableMeta>> tables_;
	std::unordered_map<std::string, table_oid_t> table_names_;
};
} // namespace db
