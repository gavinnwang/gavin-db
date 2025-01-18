#pragma once

#include "storage/buffer/buffer_pool.hpp"
#include "meta/catalog.hpp"

namespace db {
class ExecutorContext {
public:
	ExecutorContext(Catalog &catalog, BufferPool &bpm) : catalog {catalog}, bpm_ {bpm} {
	}

	~ExecutorContext() = default;

	DISALLOW_COPY_AND_MOVE(ExecutorContext);

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
