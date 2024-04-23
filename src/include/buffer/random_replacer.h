#pragma once

#include "buffer/replacer.hpp"
#include "common/config.hpp"
#include <cstdint>
#include <unordered_map>

namespace db {
class RandomBogoReplacer : public Replacer {
  public: 
    explicit RandomBogoReplacer(size_t num_frames);
    ~RandomBogoReplacer() = default;
    auto Evict(frame_id_t *frame_id) -> bool;
    void Pin(frame_id_t frame_id);
    void Unpin(frame_id_t frame_id);
  private:
    std::unordered_map<frame_id_t, uint16_t> frame_store_; 
};
}
