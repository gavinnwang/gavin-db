#pragma once

#include <cstdint>
#include <string>
namespace db {
class KVPage {
public:
  void Init();
  auto GetNumKeys() const -> uint32_t {return num_keys_;}
  void Get(const std::string& key,std::string* value) const;
  void Put(const std::string& key, const std::string& value);
  auto GetNextTupleOffset(const std::string& key, const std::string& value) const -> std::optional<uint16_t>;
private:
  // offset + size
  using KVInfo = std::tuple<uint16_t, uint16_t>;
  KVInfo kv_info_[0];
  uint16_t num_keys_;

  static constexpr uint16_t KV_INFO_SIZE = 4;

  static_assert(sizeof(KVInfo) == KV_INFO_SIZE);
};

  static constexpr uint16_t KV_PAGE_HEADER_SIZE = 2;
  static_assert(sizeof(KVPage) == KV_PAGE_HEADER_SIZE);
}
