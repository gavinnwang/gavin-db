#pragma once

#include "common/macros.hpp"
#include "common/type.hpp"
#include "storage/serializer/deserializer.hpp"
#include "storage/serializer/serializer.hpp"

#include <string>

namespace db {
class Column {
	friend class Schema;

public:
	// to creat a dummy column
	Column() = default;

	// Explicitly default the copy operations
	Column(const Column &) = default;
	Column &operator=(const Column &) = default;

	// Explicitly default move operations
	Column(Column &&) noexcept = default; // Ensure it's noexcept
	Column &operator=(Column &&) noexcept = default;

	// fixed length col
	Column(std::string column_name, TypeId type)
	    : column_name_(std::move(column_name)), column_type_(type), length_(Type::TypeSize(type)) {
		ASSERT(type != TypeId::VARCHAR, "Wrong constructor for VARCHAR type.");
		ASSERT(length_ > 0, "Invalid column length");
	}

	// variable length col
	Column(std::string column_name, TypeId type, uint32_t length)
	    : column_name_(std::move(column_name)), column_type_(type), length_(Type::TypeSize(type, length)) {
		ASSERT(type == TypeId::VARCHAR, "Wrong constructor for non-VARCHAR type.");
		ASSERT(length_ > 0, "Invalid column length");
	}

	auto GetName() const -> std::string {
		return column_name_;
	}
	auto GetStorageSize() const -> uint32_t {
		return length_;
	}
	auto GetOffset() const -> uint32_t {
		return column_offset_;
	}
	auto GetType() const -> TypeId {
		return column_type_;
	}
	auto IsInlined() const -> bool {
		return column_type_ != TypeId::VARCHAR;
	}
	auto ToString(bool simplified = true) const -> std::string;
	// void SerializeTo(char *storage) const;
	// void DeserializeFrom(const char *storage);
	// auto GetSerializationSize() const -> uint32_t;

	void Serialize(Serializer &serializer) const {
		serializer.WriteProperty(100, "column_name", column_name_);
		serializer.WriteProperty(101, "column_type", column_type_);
		serializer.WriteProperty(102, "length", length_);
		serializer.WriteProperty(103, "column_offset", column_offset_);
	}
	static Column Deserialize(Deserializer &deserializer) {
		auto result = Column();
		deserializer.ReadProperty(100, "column_name", result.column_name_);
		deserializer.ReadProperty(101, "column_type", result.column_type_);
		deserializer.ReadProperty(102, "length", result.length_);
		deserializer.ReadProperty(103, "column_offset", result.column_offset_);
		return result;
	}

private:
	std::string column_name_;
	TypeId column_type_;
	// size of col
	uint32_t length_;
	// offset in the tuple
	uint32_t column_offset_ = 0;
};
} // namespace db
