#pragma once
#include "storage/kv_page.hpp"
#include "storage/disk.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <optional>
#include <string>
#include <vector>
namespace db{
void KVPage::Init(){
  num_keys_ = 0; 
}

void KVPage::Get(const std::string& key, std::string* value) const {
  
  return;  
}

  
void KVPage::Put(const std::string& key, const std::string& value) {
  auto offset = GetNextTupleOffset(key, value);
  if (offset == std::nullopt) {
   return; 
  }
  uint16_t kv_size = key.size() + value.size();
  if (key.size() > std::numeric_limits<uint16_t>::max()) {
    std::cout << "not allowed to have a key greater than max uint16" << std::endl;
    return;
  } 
  auto meta = KVMeta{false, static_cast<uint16_t>(key.size())};
  kv_info_[num_keys_] = std::make_tuple(*offset, kv_size, meta);
  num_keys_++;
  memcpy(page_start_ + *offset, key.data(), key.size());
  memcpy(page_start_ + *offset + key.size(), key.data(), key.size());
}

auto KVPage::GetNextTupleOffset(const std::string& key, const std::string& value) const -> std::optional<uint16_t> {
  size_t end_offset;
  if (num_keys_ > 0) {
    auto &[offset, size, meta] = kv_info_[num_keys_ - 1];
    end_offset = offset;
  } else {
    end_offset = PAGE_SIZE;
  }
  uint16_t kv_size = key.size() + value.size();
  auto kv_offset = end_offset - kv_size;
  auto offset_size = KV_PAGE_HEADER_SIZE + KV_INFO_SIZE * (num_keys_ + 1);

  if (kv_offset < offset_size) {
    return std::nullopt;
  }
  return kv_offset;
}

void KVPage::PrintContent() const {
  for (const auto& [offset, size, meta] : kv_info_) {
    std::vector<char> data;
    std::cout<< "[" << offset << ", " << size << ", " << meta.key_size_ << "]" << std::endl;
    memmove(data.data(), page_start_ + offset, size);
    std::string str(data.begin(), data.end()); 
    std:: cout << str << std::endl;
  }
}
}
