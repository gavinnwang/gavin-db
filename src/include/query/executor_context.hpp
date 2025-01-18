#pragma once

#include "meta/catalog.hpp"
#include "storage/buffer/buffer_pool.hpp"

namespace db {
class ExecutorContext {
public:
	ExecutorContext(Catalog &catalog, BufferPool &bpm) : catalog {catalog}, bpm_ {bpm} {
	}

	~ExecutorContext() = default;

	ExecutorContext(const ExecutorContext &) = delete;
	ExecutorContext &operator=(const ExecutorContext &) = delete;
	ExecutorContext(ExecutorContext &&) = delete;
	ExecutorContext &operator=(ExecutorContext &&) = delete;

	[[nodiscard]] Catalog &GetCatalog() const {
		return catalog;
	}

	[[nodiscard]] BufferPool &GetBufferPoolManager() const {
		return bpm_;
	}

private:
	Catalog &catalog;
	BufferPool &bpm_;
};
} // namespace db
