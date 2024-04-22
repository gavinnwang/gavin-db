#pragma once

#include <cstdint>
#include <string>
namespace db {
static constexpr uint16_t KV_META_SIZE = 4;
struct KVMeta {
  bool is_deleted_;
  uint16_t key_size_;
};
static_assert(sizeof(KVMeta) == KV_META_SIZE);

static constexpr uint64_t KV_PAGE_HEADER_SIZE = 2;

class KVPage {
public:
  void Init();
  auto GetNumKeys() const -> uint32_t {return num_keys_;}
  void Get(const std::string& key,std::string* value) const;
  void Put(const std::string& key, const std::string& value);
  auto GetNextTupleOffset(const std::string& key, const std::string& value) const -> std::optional<uint16_t>;
  void PrintContent() const;
private:
  // offset + size
  using KVInfo = std::tuple<uint16_t, uint16_t, KVMeta>;
  char page_start_[0];
  KVInfo kv_info_[0];
  uint16_t num_keys_;
  
  static constexpr uint16_t KV_INFO_SIZE = 2 + KV_PAGE_HEADER_SIZE + KV_META_SIZE;
  static_assert(sizeof(KVInfo) == KV_INFO_SIZE);
};

  static_assert(sizeof(KVPage) == KV_PAGE_HEADER_SIZE);
}
