#pragma once
#include "catalog/schema.hpp"
#include "common/rid.hpp"
#include "common/value.hpp"
namespace db {
class Tuple {

public:
  // Default constructor (to create a dummy tuple)
  Tuple() = default;

  // construct a tuple using input values
  Tuple(std::vector<Value> values, const Schema *schema);

  Tuple(const Tuple &other) = default;
  // move constructor
  Tuple(Tuple &&other) noexcept = default;
  // assign operator, deep copy
  auto operator=(const Tuple &other) -> Tuple & = default;
  // serialize tuple data
  void SerializeTo(char *storage) const;
  // deserialize tuple data(deep copy)
  void DeserializeFrom(const char *storage);

  // return RID of current tuple
  inline auto SetRid(RID rid) { rid_ = rid; }

  // Get the address of this tuple in the table's backing store
  inline auto GetData() const -> const char * { return data_.data(); }

  // Get length of the tuple, including varchar length
  inline auto GetStorageSize() const -> uint32_t { return data_.size(); }

private:
  // Get the starting storage address of specific column
  auto GetDataPtr(const Schema &schema, uint32_t column_idx) const -> const
      char *;

  RID rid_{};
  // char *data_;
  std::vector<char> data_;
};
} // namespace db
