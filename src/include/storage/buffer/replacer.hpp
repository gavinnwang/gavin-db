#pragma once

#include "common/typedef.hpp"
namespace db {
class Replacer {
public:
	explicit Replacer() = default;
	virtual ~Replacer() = default;
	Replacer(const Replacer &) = delete;
	Replacer &operator=(const Replacer &) = delete;
	Replacer(Replacer &&) = delete;
	Replacer &operator=(Replacer &&) = delete;
	virtual auto Evict(frame_id_t &frame_id) -> bool = 0;
	virtual void Pin(frame_id_t frame_id) = 0;
	virtual void Unpin(frame_id_t frame_id) = 0;
	virtual void Print() = 0;
};
} // namespace db
