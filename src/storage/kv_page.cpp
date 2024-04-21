#pragma once
#include "storage/kv_page.hpp"
#include "storage/disk.hpp"
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
namespace db{
void KVPage::Init(){
  num_keys_ = 0; 
}

void KVPage::Get(const std::string& key, std::string* value) const {
  return;  
}

  void KVPage::Put(const std::string& key, const std::string& value){

  auto offset = GetNextTupleOffset(key, value);
  if (offset == std::nullopt) {
   return; 
  }
  uint16_t kv_size = key.size() + value.size();
  kv_info_[num_keys_] = std::make_tuple(*offset, kv_size);
  num_keys_++;
  memcpy(page_start_ + *tuple_offset, tuple.data_.data(), tuple.GetLength());
  return tuple_id;


}

auto KVPage::GetNextTupleOffset(const std::string& key, const std::string& value) const -> std::optional<uint16_t> {
  size_t end_offset;
  if (num_keys_ > 0) {
    auto &[offset, size] = kv_info_[num_keys_ - 1];
    end_offset = offset;
  } else {
    end_offset = PAGE_SIZE;
  }
  auto kv_size = key.size() + value.size();
  auto kv_offset = end_offset - kv_size;
  auto offset_size = KV_PAGE_HEADER_SIZE + KV_INFO_SIZE * (num_keys_ + 1);

  if (kv_offset < offset_size) {
    return std::nullopt;
  }
  return kv_offset;
}
}
