#pragma once
#include "catalog/schema.hpp"
#include "common/config.hpp"
#include "storage/page_allocator.hpp"
#include "storage/serializer/serializer.hpp"

#include <cstdint>
#include <memory>
#include <mutex>

namespace db {

struct TableMeta : public PageAllocator {
	explicit TableMeta() = default;

	TableMeta(Schema schema, std::string name, table_oid_t table_oid)
	    : schema_ {std::move(schema)}, name_ {std::move(name)}, table_oid_ {table_oid} {
	}

	void Serialize(Serializer &serializer) const {
		serializer.WriteProperty(100, "table_name", name_);
		serializer.WriteProperty(101, "table_oid", table_oid_);
		serializer.WriteProperty(102, "table_schema", schema_);
		serializer.WriteProperty(103, "last_table_page_id", last_table_page_id_);
	}

	[[nodiscard]] static std::unique_ptr<TableMeta> Deserialize(Deserializer &deserializer) {
		auto meta = std::make_unique<TableMeta>();
		deserializer.ReadProperty(100, "table_name", meta->name_);
		deserializer.ReadProperty(101, "table_oid", meta->table_oid_);
		deserializer.ReadProperty(102, "table_schema", meta->schema_);
		deserializer.ReadProperty(103, "last_table_page_id", meta->last_table_page_id_);
		return meta;
	}

	[[nodiscard]] inline auto GetLastTablePageId() const -> page_id_t {
		return last_table_page_id_;
	}

	inline void SetLastTablePageId(page_id_t last_table_page_id) {
		last_table_page_id_ = last_table_page_id;
	}

	PageId AllocatePage() final {
		last_table_page_id_++;
		return {table_oid_, last_table_page_id_};
	}

	void IncreaseTupleCount() {
		tuple_count_++;
	}

	Schema schema_;
	std::string name_;
	table_oid_t table_oid_ {INVALID_TABLE_OID};
	page_id_t last_table_page_id_ {INVALID_PAGE_ID};
	uint64_t tuple_count_ {0};
  std::mutex latch_;
};
} // namespace db
