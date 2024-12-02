#pragma once

#include "buffer/buffer_pool_manager.hpp"
#include "catalog/catalog_manager.hpp"

namespace db {
class ExecutorContext {
public:
	ExecutorContext(CatalogManager &catalog, BufferPoolManager &bpm) : catalog_ {catalog}, bpm_ {bpm} {
	}

	~ExecutorContext() = default;

	DISALLOW_COPY_AND_MOVE(ExecutorContext);

	CatalogManager &GetCatalog() const {
		return catalog_;
	}

	BufferPoolManager &GetBufferPoolManager() const {
		return bpm_;
	}

private:
	CatalogManager &catalog_;
	BufferPoolManager &bpm_;
};
} // namespace db
