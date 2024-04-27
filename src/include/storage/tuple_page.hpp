#pragma once
#include "common/config.hpp"
#include "storage/tuple.hpp"
#include <cstdint>
#include <tuple>
namespace db {
class TablePage {
public:
  void Init();
  auto GetNumTuples() const -> uint32_t { return num_tuples_; }

private:
  //
  using TupleInfo = std::tuple<uint16_t, uint16_t, TupleMeta>;
  char page_start_[0];
  page_id_t next_page_id_;
  uint16_t num_tuples_;
  uint16_t num_deleted_tuples_;
  TupleInfo tuple_info_[0];

  static constexpr size_t TUPLE_INFO_SIZE = 6;
  static_assert(sizeof(TupleInfo) == TUPLE_INFO_SIZE);
};
} // namespace db
