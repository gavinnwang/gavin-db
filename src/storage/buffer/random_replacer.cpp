#include "storage/buffer/random_replacer.h"
namespace db {
RandomBogoReplacer::RandomBogoReplacer() {
}
auto RandomBogoReplacer::Evict(frame_id_t &frame_id) -> bool {
	for (auto &iter : frame_store_) {
		if (iter.second) {
			frame_id = iter.first;
			// frame_store_.erase(iter.first);
			// change to not evitctable
			iter.second = false;
			return true;
		}
	}
	return false;
}
void RandomBogoReplacer::Pin(frame_id_t frame_id) {
	auto it = frame_store_.find(frame_id);
	if (it != frame_store_.end()) {
		it->second = false;
	} else {
		frame_store_.insert({frame_id, false});
	}
}
void RandomBogoReplacer::Unpin(frame_id_t frame_id) {
	auto it = frame_store_.find(frame_id);
	if (it != frame_store_.end()) {
		it->second = true;
	}
}
} // namespace db
