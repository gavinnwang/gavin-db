#include "meta/catalog_manager.hpp"

#include "common/exception.hpp"
#include "common/typedef.hpp"
#include "index/bplus_tree_index.hpp"
#include "index/index.hpp"
#include "storage/file_path_manager.hpp"

#include <optional>

namespace db {

std::optional<table_oid_t> CatalogManager::CreateTable(const std::string &table_name, const Schema &schema) {
	if (table_names_.contains(table_name)) {
		return std::nullopt;
	}
	const table_oid_t table_oid = tables_.size();
	table_names_.emplace(table_name, table_oid);

	auto table_meta = std::make_unique<TableMeta>(schema, table_name, table_oid);

	tables_.insert({table_oid, std::move(table_meta)});
	index_names_.emplace(table_name, std::unordered_map<std::string, index_oid_t> {});

	CreateFileIfNotExists(FilePathManager::GetInstance().GetTableMetaPath(table_name));
	CreateFileIfNotExists(FilePathManager::GetInstance().GetTableDataPath(table_name));
	PersistToDisk();
	// create table data and meta files
	return table_oid;
}

std::optional<index_oid_t> CatalogManager::CreateIndex(const std::string &index_name, const std::string &table_name,
                                                       const Column &key_col, bool is_primary_key, IndexType index_type,
                                                       BufferPool &bpm) {
	if (table_names_.find(table_name) == table_names_.end()) {
		return std::nullopt;
	}
	// check if index already exists
	auto &table_indexes = index_names_.find(table_name)->second;
	if (table_indexes.find(index_name) != table_indexes.end()) {
		// The requested index already exists for this table
		return std::nullopt;
	}
	table_oid_t table_id = table_names_[table_name];

	// IndexMeta(std::string name, table_oid_t table_id, Column key_col, IndexConstraintType index_constraint_type)
	IndexConstraintType constraint_type = is_primary_key ? IndexConstraintType::PRIMARY : IndexConstraintType::NONE;
	auto index_meta = std::make_unique<IndexMeta>(index_name, table_id, key_col, constraint_type, index_type);

	std::unique_ptr<Index> index;
	const auto &table_meta = tables_.at(table_names_.at(table_name));
	if (index_type == IndexType::BPlusTreeIndex) {
		auto btree_index = std::make_unique<BTreeIndex>(*index_meta, *table_meta, bpm);
	} else {
		throw NotImplementedException("Unsupported index type");
	}

	const index_oid_t index_oid = indexes_.size();

	indexes_.emplace(index_oid, std::move(index_meta));
	table_indexes.emplace(index_name, index_oid);
	return index_oid;
}
} // namespace db
