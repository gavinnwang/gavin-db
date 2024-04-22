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

auto KVPage::Get(const std::string& key, std::string* value) const -> bool{
  for (int i = 0; i < num_keys_; i++) {
    const auto& [offset, size, meta] = kv_info_[i];
    std::vector<char> key_buf (meta.key_size_);
    memmove(key_buf.data(), page_start_ + offset, meta.key_size_);
    std::string key_str(key_buf.begin(), key_buf.end()); 
    if (key_str == key) {
      std::vector<char> val_buf (size - meta.key_size_);
      memmove(val_buf.data(), page_start_ + offset + meta.key_size_, size - meta.key_size_);
      std::string val_str(val_buf.begin(), val_buf.end());
      if (value != nullptr) {
        *value = val_str;
      }
      return true;
    }
  }
  return false;  
}

auto KVPage::Put(const std::string& key, const std::string& value) -> bool {
  auto offset = GetNextTupleOffset(key, value);
  if (offset == std::nullopt) {
   return false; 
  }
  uint16_t kv_size = key.size() + value.size();
  if (key.size() > std::numeric_limits<uint16_t>::max()) {
    std::cout << "not allowed to have a key greater than max uint16" << std::endl;
    return false;
  } 
  if (Get(key, nullptr)) {
    std::cout << "key already exists" << std::endl;
    return false;
  }
  auto meta = KVMeta{false, static_cast<uint16_t>(key.size())};
  kv_info_[num_keys_] = std::make_tuple(*offset, kv_size, meta);
  num_keys_++;
  memcpy(page_start_ + *offset, key.data(), key.size());
  memcpy(page_start_ + *offset + key.size(), value.data(), value.size());
  return true;
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
  for (int i = 0; i < num_keys_; i++) {
  const auto& [offset, size, meta] = kv_info_[i];
  // for (const auto& [offset, size, meta] : kv_info_) {
    // std::vector<char> data (size);
    // char key_buf[meta.key_size_];
    // char val_buf[size - meta.key_size_];
    std::vector<char> key_buf (meta.key_size_);
    std::vector<char> val_buf (size - meta.key_size_);
    // std::cout<< "[" << offset << ", " << size << ", " << meta.key_size_ << "]" << std::endl;
    memmove(key_buf.data(), page_start_ + offset, meta.key_size_);
    memmove(val_buf.data(), page_start_ + offset + meta.key_size_, size - meta.key_size_);
    std::string key_str(key_buf.begin(), key_buf.end()); 
    std::string val_str(val_buf.begin(), val_buf.end());
    // std:: cout << key_str << std::endl;
    // std:: cout << val_str << std::endl;
    std::cout << "(" + key_str + ": " + val_str + ")" <<std::endl;
  }
}
}
