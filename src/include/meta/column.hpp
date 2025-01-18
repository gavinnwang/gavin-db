#pragma once

#include "common/type.hpp"
#include "fmt/format.h"
#include "storage/serializer/deserializer.hpp"
#include "storage/serializer/serializer.hpp"

#include <cassert>
#include <string>

namespace db {
class Column {
	friend class Schema;

public:
	// to creat a dummy column
	Column() = default;

	// fixed length col
	Column(std::string column_name, TypeId type)
	    : column_name_(std::move(column_name)), column_type_(type), length_(Type::TypeSize(type)) {
		assert(type != TypeId::VARCHAR && "Wrong constructor for VARCHAR type.");
		assert(length_ > 0 && "Invalid column length");
	}

	// variable length col
	Column(std::string column_name, TypeId type, uint32_t length)
	    : column_name_(std::move(column_name)), column_type_(type), length_(Type::TypeSize(type, length)) {
		assert(type == TypeId::VARCHAR && "Wrong constructor for non-VARCHAR type.");
		assert(length_ > 0 && "Invalid column length");
	}

	[[nodiscard]] std::string GetName() const {
		return column_name_;
	}
	[[nodiscard]] uint32_t GetStorageSize() const {
		return length_;
	}
	[[nodiscard]] uint32_t GetStorageOffset() const {
		return storage_offset_;
	}
	[[nodiscard]] uint32_t GetSchemaOffset() const {
		return schema_offset_;
	}
	[[nodiscard]] TypeId GetType() const {
		return column_type_;
	}
	[[nodiscard]] bool IsInlined() const {
		return column_type_ != TypeId::VARCHAR;
	}
	[[nodiscard]] std::string ToString(bool simplified = true) const;

	void Serialize(Serializer &serializer) const {
		serializer.WriteProperty(100, "column_name", column_name_);
		serializer.WriteProperty(101, "column_type", column_type_);
		serializer.WriteProperty(102, "length", length_);
		serializer.WriteProperty(103, "column_offset", storage_offset_);
	}
	static Column Deserialize(Deserializer &deserializer) {
		auto result = Column();
		deserializer.ReadProperty(100, "column_name", result.column_name_);
		deserializer.ReadProperty(101, "column_type", result.column_type_);
		deserializer.ReadProperty(102, "length", result.length_);
		deserializer.ReadProperty(103, "column_offset", result.storage_offset_);
		return result;
	}

private:
	// name of col
	std::string column_name_;
	// type of col
	TypeId column_type_;
	// size of col
	uint32_t length_;
	// storage offset
	uint32_t storage_offset_ = 0;
	// schema offset
	idx_t schema_offset_ = 0;
	// if auto increment col
	bool is_auto_increment_ = false;
};
} // namespace db

template <>
struct fmt::formatter<db::Column> : formatter<std::string_view> {
	auto format(const db::Column &x, format_context &ctx) const {
		return formatter<string_view>::format(x.ToString(), ctx);
	}
};
