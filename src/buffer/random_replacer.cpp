#include "buffer/random_replacer.h"
namespace db{
  RandomBogoReplacer::RandomBogoReplacer(size_t num_frames) {}

  auto RandomBogoReplacer::Evict(frame_id_t *frame_id) -> bool {
    for (const auto& it : frame_store_) {
      if (it.second) {
        *frame_id = it.first;
        frame_store_.erase(it.first);
        return true;
      }
    }
    return false;
  }
  void RandomBogoReplacer::Pin(frame_id_t frame_id) {
    auto it = frame_store_.find(frame_id);
    if (it != frame_store_.end()) {
      it->second = true;
    }
  }
  void RandomBogoReplacer::Unpin(frame_id_t frame_id) {
    auto it = frame_store_.find(frame_id);
    if (it != frame_store_.end()) {
      it->second = false;
    }
  }
}
