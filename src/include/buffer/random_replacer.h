#pragma once

#include "buffer/replacer.hpp"
#include "common/config.hpp"
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
    // frame id, pinned state
    std::unordered_map<frame_id_t, bool> used_frames_; 
};
}
