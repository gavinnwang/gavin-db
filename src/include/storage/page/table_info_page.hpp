
#pragma once
#include "storage/table/table_meta.hpp"

#include <cstdint>
namespace db {
static constexpr uint64_t TABLE_META_PAGE_HEADER_SIZE = 12;

class TableMetaPage {
	// stores the schema and table name info to disk
	friend class TablePage;

public:
	TableMetaPage() = delete;
	~TableMetaPage() = delete;
	void Init(const std::string name, const Schema &schema, const table_oid_t table_oid);
	auto GetTableMeta() const -> TableMeta;
	void UpdateTableSchema(const Schema &schema);
	inline auto GetFirstTablePageId() const -> page_id_t {
		return first_table_page_id_;
	}
	inline auto GetLastTablePageId() const -> page_id_t {
		return last_table_page_id_;
	}
	inline void SetFirstTablePageId(page_id_t first_table_page_id) {
		first_table_page_id_ = first_table_page_id;
	}
	inline void SetLastTablePageId(page_id_t last_table_page_id) {
		last_table_page_id_ = last_table_page_id;
	}

private:
	void StoreTableMeta(const TableMeta &table_meta);
	char page_start_[0];
	page_id_t first_table_page_id_;
	page_id_t last_table_page_id_;
	uint32_t table_meta_offset_ {0};
};

static_assert(sizeof(TableMetaPage) == TABLE_META_PAGE_HEADER_SIZE);
} // namespace db
