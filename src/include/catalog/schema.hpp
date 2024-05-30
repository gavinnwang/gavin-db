#pragma once

#include "catalog/column.hpp"
#include "storage/serializer/deserializer.hpp"
#include "storage/serializer/serializer.hpp"

#include <cstdint>
#include <memory>
namespace db {

class Schema;
using SchemaRef = std::shared_ptr<const Schema>;

class Schema {
public:
	Schema() = default;
	explicit Schema(const std::vector<Column> &columns);
	auto GetColumns() const -> const std::vector<Column> & {
		return columns_;
	}
	auto GetColumn(const uint32_t col_idx) const -> const Column & {
		return columns_[col_idx];
	}
	auto TryGetColIdx(const std::string &col_name) const -> std::optional<uint32_t> {
		for (uint32_t i = 0; i < columns_.size(); ++i) {
			if (columns_[i].GetName() == col_name) {
				return std::optional {i};
			}
		}
		return std::nullopt;
	}
	auto GetColumnCount() const -> uint32_t {
		return static_cast<uint32_t>(columns_.size());
	}
	auto GetUnlinedColumns() const -> const std::vector<uint32_t> & {
		return uninlined_columns_;
	}
	auto GetUnlinedColumnCount() const -> uint32_t {
		return static_cast<uint32_t>(uninlined_columns_.size());
	}
	inline auto GetTupleInlinePartStorageSize() const -> uint32_t {
		return tuple_inline_part_storage_size_;
	}
	void SerializeTo(char *storage) const;
	void DeserializeFrom(const char *storage);

	void Serialize(Serializer &serializer) const {
		serializer.WriteProperty(100, "columns", columns_);
		serializer.WriteProperty(101, "uninlined_columns", uninlined_columns_);
		serializer.WriteProperty(102, "tuple_inline_part_storage_size", tuple_inline_part_storage_size_);
	}
	static Schema Deserialize(Deserializer &deserializer) {
		auto result = Schema();
		deserializer.ReadProperty(100, "columns", result.columns_);
		deserializer.ReadProperty(101, "uninlined_columns", result.uninlined_columns_);
		deserializer.ReadProperty(102, "tuple_inline_part_storage_size", result.tuple_inline_part_storage_size_);
		return result;
	}

	auto GetSerializationSize() const -> uint32_t;

	auto ToString() const -> std::string {
		std::string str = "Schema(";
		for (const auto &col : columns_) {
			str += col.ToString() + ", ";
		}
		str.pop_back();
		str.pop_back();
		str += ")";
		return str;
	}

private:
	std::vector<Column> columns_;
	std::vector<uint32_t> uninlined_columns_;
	// the inline poortion of the tuple, consist of fixed length columns and the
	// pointer for var len columns
	uint32_t tuple_inline_part_storage_size_;
	// // storage size of the schema
	// uint32_t storage_size_;
};
} // namespace db
