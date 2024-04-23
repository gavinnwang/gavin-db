#pragma once

#include "common/config.hpp"
namespace db {  
class Replacer {
  public: 
    explicit Replacer() = default;
    virtual ~ Replacer() = default;
    virtual auto Evict(frame_id_t *frame_id) -> bool = 0;
    virtual void Pin(frame_id_t frame_id) = 0;
    virtual void Unpin(frame_id_t frame_id) = 0;
};
}
