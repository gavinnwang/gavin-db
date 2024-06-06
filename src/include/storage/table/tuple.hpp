#pragma once
#include "catalog/schema.hpp"
#include "common/rid.hpp"
#include "common/typedef.hpp"
#include "common/value.hpp"

#include <malloc/_malloc.h>
#include <string>
namespace db {

static constexpr size_t TUPLE_META_SIZE = 1;

struct TupleMeta {
	bool is_deleted_;

	friend auto operator==(const TupleMeta &a, const TupleMeta &b) {
		return a.is_deleted_ == b.is_deleted_;
	}

	friend auto operator!=(const TupleMeta &a, const TupleMeta &b) {
		return !(a == b);
	}
};

static_assert(sizeof(TupleMeta) == TUPLE_META_SIZE);
class Tuple {

	friend class TablePage;
	friend class TableHeap;

public:
	// Default constructor (to create a dummy tuple)
	Tuple() = default;

	// construct a tuple using input values
	Tuple(std::vector<Value> values, const Schema &schema);

	Tuple(const Tuple &other) = default;
	// move constructor
	Tuple(Tuple &&other) noexcept = default;
	// assign operator, deep copy
	auto operator=(const Tuple &other) -> Tuple & = default;
	// serialize tuple data
	void SerializeTo(char *storage) const;
	// deserialize tuple data(deep copy)
	void DeserializeFrom(const char *storage, uint32_t size);

	inline auto SetRid(RID rid) {
		rid_ = rid;
	}

	const_data_ptr_t GetData() const {
		return data_.data();
	}

	inline auto GetStorageSize() const -> uint32_t {
		return data_.size();
	}

	Value GetValue(const Column &col) const;

	Value GetValue(const Schema &schema, uint32_t column_idx) const;

	auto ToString(const Schema &schema) const -> std::string;

private:
	// Get the starting storage address of specific column
	const_data_ptr_t GetDataPtr(const Column &col) const;

	RID rid_ {};

	// char *data_;
	std::vector<data_t> data_;
};
} // namespace db
