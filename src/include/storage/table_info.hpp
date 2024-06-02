#pragma once
#include "catalog/schema.hpp"
#include "common/config.hpp"
#include "storage/serializer/serializer.hpp"

#include <cstdint>
#include <memory>

namespace db {

struct TableInfo {
	Schema schema_;
	std::string name_;
	table_oid_t table_oid_;
	// page_id_t first_table_page_id_ {INVALID_PAGE_ID};
	page_id_t last_table_page_id_ {INVALID_PAGE_ID};

	explicit TableInfo() {};

	TableInfo(Schema schema, std::string name, table_oid_t table_oid)
	    : schema_ {std::move(schema)}, name_ {std::move(name)}, table_oid_ {table_oid} {
	}

	void Serialize(Serializer &serializer) const {
		serializer.WriteProperty(100, "table_name", name_);
		serializer.WriteProperty(101, "table_oid", table_oid_);
		serializer.WriteProperty(102, "table_schema", schema_);
		// serializer.WriteProperty(103, "first_table_page_id", first_table_page_id_);
		serializer.WriteProperty(104, "last_table_page_id", last_table_page_id_);
	}

	static std::unique_ptr<TableInfo> Deserialize(Deserializer &deserializer) {
		auto info = std::make_unique<TableInfo>();
		deserializer.ReadProperty(100, "table_name", info->name_);
		deserializer.ReadProperty(101, "table_oid", info->table_oid_);
		deserializer.ReadProperty(102, "table_schema", info->schema_);
		// deserializer.ReadProperty(103, "first_table_page_id", info->first_table_page_id_);
		deserializer.ReadProperty(104, "last_table_page_id", info->last_table_page_id_);
		return info;
	}

	void SerializeTo(char *storage) const {
		uint32_t offset = 0;
		schema_.SerializeTo(storage + offset);
		offset += schema_.GetSerializationSize();
		// now serialize the table name length and string
		uint32_t table_name_size = name_.size();
		memcpy(storage + offset, &table_name_size, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(storage + offset, name_.data(), table_name_size);
		offset += table_name_size;
		memcpy(storage + offset, &table_oid_, sizeof(uint32_t));
	}

	static auto DeserializeFrom(const char *storage) -> TableInfo {
		auto schema = Schema();
		uint32_t offset = 0;
		schema.DeserializeFrom(storage + offset);
		offset += schema.GetSerializationSize();
		uint32_t table_name_size = *reinterpret_cast<const uint32_t *>(storage + offset);
		offset += sizeof(uint32_t);
		std::string table_name(storage + offset, table_name_size);
		offset += table_name_size;
		table_oid_t table_oid = *reinterpret_cast<const uint32_t *>(storage + offset);

		return {schema, table_name, table_oid};
	}

	auto GetSerializationSize() const -> uint32_t {
		auto sz = schema_.GetSerializationSize() + sizeof(uint32_t) + name_.size() + sizeof(table_oid_);
		return sz;
	}

	// inline auto GetFirstTablePageId() const -> page_id_t {
	// 	return first_table_page_id_;
	// }
	inline auto GetLastTablePageId() const -> page_id_t {
		return last_table_page_id_;
	}
	// inline void SetFirstTablePageId(page_id_t first_table_page_id) {
	// 	first_table_page_id_ = first_table_page_id;
	// }
	inline void SetLastTablePageId(page_id_t last_table_page_id) {
		last_table_page_id_ = last_table_page_id;
	}
};
} // namespace db
