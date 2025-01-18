#pragma once

#include "storage/buffer/replacer.hpp"
#include "common/config.hpp"
#include "common/logger.hpp"

#include <cstdint>
#include <unordered_map>

namespace db {
class RandomBogoReplacer : public Replacer {
public:
	explicit RandomBogoReplacer();
	~RandomBogoReplacer() override = default;
	auto Evict(frame_id_t &frame_id) -> bool override;
	void Pin(frame_id_t frame_id) override;
	void Unpin(frame_id_t frame_id) override;
	void Print() override {
		for (const auto &iter : frame_store_) {
			LOG_TRACE("frame_id: {} is_pinned: {}\n", iter.first, iter.second ? "false" : "true");
		}
	}

private:
	std::unordered_map<frame_id_t, uint16_t> frame_store_;
};
} // namespace db
