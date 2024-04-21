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
  auto GetNextTupleOffset(const std::string& key, const std::string& value);
private:
  uint32_t num_keys_;
  // offset + size
  using KVInfo = std::tuple<uint16_t, uint16_t>;
  KVInfo kv_info_[0];
  uint16_t num_tuples_;
  static constexpr size_t KV_INFO_SIZE = 4;
  static_assert(sizeof(KVInfo) == KV_INFO_SIZE);
};
}
