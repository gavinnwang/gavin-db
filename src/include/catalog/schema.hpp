#pragma once

#include "catalog/column.hpp"
#include "common/typedef.hpp"
#include "fmt/ranges.h"
#include "storage/serializer/deserializer.hpp"
#include "storage/serializer/serializer.hpp"

#include <cstdint>
#include <memory>
namespace db {

class Schema;
using SchemaRef = std::unique_ptr<const Schema>;

class Schema {
public:
	explicit Schema() = default;
	explicit Schema(const std::vector<Column> &columns);
	[[nodiscard]] const std::vector<Column> &GetColumns() const {
		return columns_;
	}
	[[nodiscard]] const Column &GetColumn(const column_t col_idx) const {
		return columns_[col_idx];
	}
	[[nodiscard]] std::optional<column_t> TryGetColIdx(const std::string &col_name) const {
		for (uint32_t i = 0; i < columns_.size(); ++i) {
			if (columns_[i].GetName() == col_name) {
				return std::optional {i};
			}
		}
		return std::nullopt;
	}
	[[nodiscard]] uint32_t GetColumnCount() const {
		return static_cast<uint32_t>(columns_.size());
	}
	[[nodiscard]] const std::vector<uint32_t> &GetUninlinedColumns() const {
		return uninlined_columns_;
	}
	[[nodiscard]] uint32_t GetUninlinedColumnCount() const {
		return static_cast<uint32_t>(uninlined_columns_.size());
	}
	[[nodiscard]] uint32_t GetTupleInlinePartStorageSize() const {
		return tuple_inline_part_storage_size_;
	}

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

	static bool TypeMatch(const Schema &a, const Schema &b) {
		return std::equal(a.GetColumns().cbegin(), a.GetColumns().cend(), b.GetColumns().cbegin(),
		                  b.GetColumns().cend(),
		                  [](auto &&col1, auto &&col2) { return col1.GetType() == col2.GetType(); });
	}

	[[nodiscard]] std::string ToString() const {
		return fmt::format("Schema({})", columns_);
	}

private:
	std::vector<Column> columns_;
	std::vector<column_t> uninlined_columns_;
	// the inline poortion of the tuple, consist of fixed length columns and the
	// pointer for var len columns
	uint32_t tuple_inline_part_storage_size_;
};
} // namespace db

template <>
struct fmt::formatter<db::Schema> : formatter<std::string_view> {
	auto format(db::Schema x, format_context &ctx) const {
		return formatter<string_view>::format(x.ToString(), ctx);
	}
};
