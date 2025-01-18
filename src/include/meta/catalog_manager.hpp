#pragma once

#include "storage/buffer/buffer_pool_manager.hpp"
#include "meta/schema.hpp"
#include "common/fs_utils.hpp"
#include "common/typedef.hpp"
#include "index/index.hpp"
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
		LOG_TRACE("Initializing meta manager...");
		std::filesystem::path catalog_path = FilePathManager::GetInstance().GetSystemCatalogPath();
		if (std::filesystem::exists(catalog_path)) {
			LOG_TRACE("Catalog file exists! Loading from disk...");
			auto catalog_fs = FileStream(catalog_path);
			BinaryDeserializer deserializer(catalog_fs);
			Deserialize(deserializer);
		} else {
			LOG_TRACE("Catalog file not found. Creating one to disk");
			PersistToDisk();
		}
		EnsureTableFilesExist();
	}

	std::optional<table_oid_t> CreateTable(const std::string &table_name, const Schema &schema);
	std::optional<index_oid_t> CreateIndex(const std::string &index_name, const std::string &table_name,
	                                       const Column &key_col, bool is_primary_key, IndexType index_type,
	                                       BufferPoolManager &bpm);

	[[nodiscard]] TableMeta &GetTableByName(const std::string &table_name) const {
		if (table_names_.find(table_name) == table_names_.end()) {
			throw Exception("Table not found when getting table info");
		}
		return *tables_.at(table_names_.at(table_name));
	};

	[[nodiscard]] TableMeta &GetTable(const table_oid_t table_oid) const {
		return GetTableByName(GetTableName(table_oid));
	}

	[[nodiscard]] std::string &GetTableName(const table_oid_t table_oid) const {
		if (tables_.find(table_oid) == tables_.end()) {
			throw Exception("Table not found when getting table name");
		}
		return tables_.at(table_oid)->name_;
	}

	void PersistToDisk() {
		LOG_TRACE("Persisting meta to disk");
		// persist the meta to disk
		std::filesystem::path catalog_path = FilePathManager::GetInstance().GetSystemCatalogPath();
		CreateFileIfNotExists(catalog_path);
		auto catalog_fs = FileStream(catalog_path);
		BinarySerializer serializer(catalog_fs);
		Serialize(serializer);

		// persist all table meta to disk
		for (const auto &[table_oid, table_meta] : tables_) {
			LOG_TRACE("Persisting table {}", table_meta->ToString());
			std::filesystem::path table_meta_path = FilePathManager::GetInstance().GetTableMetaPath(table_meta->name_);
			// CreateFileIfNotExists(table_meta_path);
			auto table_meta_fs = FileStream(table_meta_path);
			BinarySerializer table_meta_serializer(table_meta_fs);
			table_meta->Serialize(table_meta_serializer);
		}
	}

	void Serialize(Serializer &serializer) const {
		serializer.WritePropertyWithDefault(100, "tables", tables_,
		                                    std::unordered_map<table_oid_t, std::unique_ptr<TableMeta>>());

		serializer.WritePropertyWithDefault(101, "table_names", table_names_,
		                                    std::unordered_map<std::string, table_oid_t>());

		serializer.WritePropertyWithDefault(102, "indexes", indexes_,
		                                    std::unordered_map<index_oid_t, std::unique_ptr<IndexMeta>>());

		serializer.WritePropertyWithDefault(
		    103, "index_names", index_names_,
		    std::unordered_map<std::string, std::unordered_map<std::string, index_oid_t>>());

		serializer.WriteProperty(104, "magic_bytes", magic_bytes_);
	}

	void Deserialize(Deserializer &deserializer) {
		deserializer.ReadPropertyWithDefault(100, "tables", tables_,
		                                     std::unordered_map<table_oid_t, std::unique_ptr<TableMeta>>());

		deserializer.ReadPropertyWithDefault(101, "table_names", table_names_,
		                                     std::unordered_map<std::string, table_oid_t>());

		deserializer.ReadPropertyWithDefault(102, "indexes", indexes_,
		                                     std::unordered_map<index_oid_t, std::unique_ptr<IndexMeta>>());

		deserializer.ReadPropertyWithDefault(
		    103, "index_names", index_names_,
		    std::unordered_map<std::string, std::unordered_map<std::string, index_oid_t>>());

		std::string deserialized_magic_bytes;
		deserializer.ReadProperty(104, "magic_bytes", deserialized_magic_bytes);

		if (deserialized_magic_bytes != magic_bytes_) {
			throw std::runtime_error("Catalog file is corrupted or incompatible.");
		}
	}

private:
	void EnsureTableFilesExist() {
		for (const auto &[table_name, table_oid] : table_names_) {
			LOG_TRACE("Checking table files for table {}", table_name);
			auto data_exists = std::filesystem::exists(FilePathManager::GetInstance().GetTableDataPath(table_name));
			auto meta_exists = std::filesystem::exists(FilePathManager::GetInstance().GetTableMetaPath(table_name));
			if (!data_exists || !meta_exists) {
				throw Exception("Data file or meta file not found for table " + table_name);
			}
			LOG_TRACE("Check success: table {} exists", table_name);
		}
	};
	std::unordered_map<table_oid_t, std::unique_ptr<TableMeta>> tables_;
	std::unordered_map<index_oid_t, std::unique_ptr<IndexMeta>> indexes_;
	// table name -> index name -> index id
	std::unordered_map<std::string, std::unordered_map<std::string, index_oid_t>> index_names_;
	std::unordered_map<std::string, table_oid_t> table_names_;
	// magic bytes to ensure meta manager is not corrupt
	std::string magic_bytes_ {"GAVINDB_CATALOG_MANAGER"};
};
} // namespace db
