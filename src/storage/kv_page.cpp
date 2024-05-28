#include "storage/kv_page.hpp"

#include "storage/disk_manager.hpp"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <optional>
#include <string>
namespace db {

void KVPage::Init() {
	num_keys_ = 0;
	deleted_keys = 0;
}

auto KVPage::Get(const std::string &key, std::string *value) const -> bool {
	for (int i = 0; i < num_keys_; i++) {
		const auto &[offset, key_size, val_size, meta] = kv_info_[i];
		if (meta.is_deleted_) {
			continue;
		}
		std::string key_str(page_start_ + offset, key_size);
		if (key_str == key) {
			std::string val_str(page_start_ + offset + key_size, val_size);
			*value = val_str;
			return true;
		}
	}
	*value = "key doesn't exist";
	return false;
}

auto KVPage::Put(const std::string &key, const std::string &value) -> bool {
	auto offset = GetNextTupleOffset(key, value);
	if (offset == std::nullopt) {
		return false;
	}
	if (key.size() > std::numeric_limits<uint16_t>::max()) {
		std::cout << "not allowed to have a key with size greater than max uint16" << std::endl;
		return false;
	}
	std::string temp_val;
	if (Get(key, &temp_val)) {
		std::cerr << "key already exists";
		return false;
	}
	auto meta = KVMeta {false};
	kv_info_[num_keys_] = std::make_tuple(*offset, key.size(), value.size(), meta);
	num_keys_++;
	memcpy(page_start_ + *offset, key.data(), key.size());
	memcpy(page_start_ + *offset + key.size(), value.data(), value.size());
	return true;
}

auto KVPage::Delete(const std::string &key) -> bool {
	for (int i = 0; i < num_keys_; i++) {
		auto &[offset, key_size, val_size, meta] = kv_info_[i];
		if (meta.is_deleted_) {
			continue;
		}
		std::string key_str(page_start_ + offset, key_size);
		if (key_str == key) {
			meta.is_deleted_ = true;
			// num_keys_--;
			deleted_keys++;
			return true;
		}
	}
	std::cout << "key not found";
	return false;
}

auto KVPage::GetNextTupleOffset(const std::string &key, const std::string &value) const -> std::optional<uint16_t> {
	size_t end_offset;
	if (num_keys_ > 0) {
		auto &[offset, key_size, val_size, meta] = kv_info_[num_keys_ - 1];
		end_offset = offset;
	} else {
		end_offset = PAGE_SIZE;
	}
	uint16_t kv_size = key.size() + value.size();
	auto kv_offset = end_offset - kv_size;
	auto offset_size = KV_PAGE_HEADER_SIZE + KV_INFO_SIZE * (num_keys_ + 1);
	if (kv_offset < offset_size) {
		return std::nullopt;
	}
	return kv_offset;
}

void KVPage::PrintContent() const {
	for (int i = 0; i < num_keys_; i++) {
		const auto &[offset, key_size, val_size, meta] = kv_info_[i];
		std::string key_str(page_start_ + offset, key_size);
		std::string val_str(page_start_ + offset + key_size, val_size);
		if (i) {
			std::cout << std::endl;
		}
		std::cout << "(" + key_str + " : " + val_str + ")";
		if (meta.is_deleted_) {
			std::cout << " deleted";
		}
	}
}

void KVPage::Compact() {
	if (deleted_keys == 0) {
		std::cout << "no deleted kv in page, no need to do compaction";
		return;
	}

	const auto &[first_offset, first_key_size, first_val_size, _] = kv_info_[0];
	// have last offset as the start of the last kv, default is end of page
	uint16_t last_offset = first_offset + first_key_size + first_val_size;
	// pointer for moving kv info array
	uint16_t last_inserted = 0;
	for (uint16_t cur_i = 0; cur_i < num_keys_ + deleted_keys; cur_i++) {
		auto &[cur_offset, key_size, val_size, meta] = kv_info_[cur_i];
		if (meta.is_deleted_) {
			continue;
		}
		uint16_t kv_size = key_size + val_size;
		memcpy(page_start_ + last_offset - kv_size, page_start_ + cur_offset, kv_size);
		// set last offset to the start of the moved kv
		last_offset -= kv_size;
		// set the offset to the new offset
		cur_offset = last_offset;
		// shift the kv info array
		kv_info_[last_inserted] = kv_info_[cur_i];
		// shift pointer
		last_inserted++;
	}
	num_keys_ -= deleted_keys;
	deleted_keys = 0;
	PrintContent();
}
} // namespace db
