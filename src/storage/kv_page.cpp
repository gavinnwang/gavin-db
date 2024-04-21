#pragma once
#include "storage/kv_page.hpp"
namespace db{
void KVPage::Init(){
  num_keys_ = 0; 
}

void KVPage::Get(const std::string& key, std::string* value) const {
  return;  
}

  auto KVPage::GetNextTupleOffset(const TupleMeta &meta, const Tuple &tuple) const -> std::optional<uint16_t>;
}
