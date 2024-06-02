#pragma once

#include <cstdint>
#include <optional>
#include <string>

/* NOT USED  */

namespace db {
static constexpr uint16_t KV_META_SIZE = 1;
struct KVMeta {
	bool is_deleted_;
};
static_assert(sizeof(KVMeta) == KV_META_SIZE);

static constexpr uint64_t KV_PAGE_HEADER_SIZE = 4;

class KVPage {
public:
	void Init();
	auto GetNumKeys() const -> uint32_t {
		return num_keys_;
	}
	auto Get(const std::string &key, std::string *value) const -> bool;
	auto Put(const std::string &key, const std::string &value) -> bool;
	auto Delete(const std::string &key) -> bool;
	auto GetNextTupleOffset(const std::string &key, const std::string &value) const -> std::optional<uint16_t>;
	void PrintContent() const;
	void Compact();

private:
	// offset + key size + val size + kv meta
	using KVInfo = std::tuple<uint16_t, uint16_t, uint16_t, KVMeta>;
	char page_start_[0];
	// nums_keys must be before the kv info array because kv info array will grow
	// and nums keys will get overwritten by kv info array if you have it after.
	uint16_t num_keys_;
	uint16_t deleted_keys;
	KVInfo kv_info_[0];

	static constexpr uint16_t KV_INFO_SIZE = 2 + 2 + 2 + KV_META_SIZE + 1;
	static_assert(sizeof(KVInfo) == KV_INFO_SIZE);
};

static_assert(sizeof(KVPage) == KV_PAGE_HEADER_SIZE);
} // namespace db
