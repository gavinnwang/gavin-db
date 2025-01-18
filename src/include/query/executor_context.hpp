#pragma once

#include "storage/buffer/buffer_pool.hpp"
#include "meta/catalog_manager.hpp"

namespace db {
class ExecutorContext {
public:
	ExecutorContext(CatalogManager &catalog, BufferPool &bpm) : catalog_ {catalog}, bpm_ {bpm} {
	}

	~ExecutorContext() = default;

	DISALLOW_COPY_AND_MOVE(ExecutorContext);

	[[nodiscard]] CatalogManager &GetCatalog() const {
		return catalog_;
	}

	[[nodiscard]] BufferPool &GetBufferPoolManager() const {
		return bpm_;
	}

private:
	CatalogManager &catalog_;
	BufferPool &bpm_;
};
} // namespace db
