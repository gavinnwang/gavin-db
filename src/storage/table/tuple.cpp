#include "storage/table/tuple.hpp"

#include "common/typedef.hpp"
#include "common/value.hpp"

namespace db {

Tuple::Tuple(std::vector<Value> values, const Schema &schema) {
	assert(values.size() == schema.GetColumnCount());

	// 1. calc the size of tuple: inline storage size + variable storage size
	uint32_t tuple_size = schema.GetTupleInlinePartStorageSize();
	assert(tuple_size > 0 && "Tuple size must be greater than 0");
	assert(tuple_size < PAGE_SIZE && "Tuple size must be less than PAGE_SIZE");
	for (const auto &uninline_col : schema.GetUninlinedColumns()) {
		auto len = values[uninline_col].GetStorageSize();

		// if the length of the value is greater than the schema max length for that
		// column, error
		if (len > schema.GetColumn(uninline_col).GetStorageSize()) {
			throw Exception("Value length exceeds schema max length for column " +
			                schema.GetColumn(uninline_col).GetName() +
			                " (max: " + std::to_string(schema.GetColumn(uninline_col).GetStorageSize()) +
			                ", actual: " + std::to_string(len) + ")");
		}
		// size of uint32_t is the size info
		tuple_size += sizeof(uint32_t) + len;
	}
	data_.resize(tuple_size);

	std::fill(data_.begin(), data_.end(), 0);

	uint32_t column_count = schema.GetColumnCount();
	// uint32_t offset = schema.GetSerializationSize();
	uint32_t offset = schema.GetTupleInlinePartStorageSize();

	for (idx_t i = 0; i < column_count; i++) {
		const auto &col = schema.GetColumn(i);
		if (!col.IsInlined()) {
			// serialize the relative offset
			*reinterpret_cast<uint32_t *>(data_.data() + col.GetStorageOffset()) = offset;
			// serialize actual varchar value, in place (size | data)
			values[i].SerializeTo(data_.data() + offset);
			offset += values[i].GetStorageSize() + sizeof(uint32_t);
		} else {
			values[i].SerializeTo(data_.data() + col.GetStorageOffset());
		}
	}
}

void Tuple::SerializeTo(char *storage) const {
	memcpy(storage, data_.data(), data_.size());
}

void Tuple::DeserializeFrom(const char *storage, uint32_t size) {
	memcpy(this->data_.data(), storage, size);
}

Value Tuple::GetValue(const Column &col) const {
	const_data_ptr_t data_ptr = GetDataPtr(col);
	return Value::DeserializeFrom(data_ptr, col.GetType());
}

Value Tuple::GetValue(const Schema &schema, uint32_t column_idx) const {
	const auto &col = schema.GetColumn(column_idx);
	return GetValue(col);
}

const_data_ptr_t Tuple::GetDataPtr(const Column &col) const {
	bool is_inlined = col.IsInlined();
	if (is_inlined) {
		assert(col.GetStorageOffset() < data_.size() && "offset out of range");
		return (data_.data() + col.GetStorageOffset());
	}

	// read the relative offset from the tuple data.
	int32_t offset = *reinterpret_cast<const int32_t *>(data_.data() + col.GetStorageOffset());
	// return the beginning address of the real data for the VARCHAR type.
	return (data_.data() + offset);
}

auto Tuple::ToString(const Schema &schema) const -> std::string {
	std::vector<std::string> values;
	uint32_t column_count = schema.GetColumnCount();

	for (uint32_t column_itr = 0; column_itr < column_count; ++column_itr) {
		Value val = GetValue(schema, column_itr);
		values.push_back(val.ToString());
	}

	return fmt::format("({})", fmt::join(values, ", "));
}
} // namespace db
