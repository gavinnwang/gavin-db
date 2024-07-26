#pragma once

#include "buffer/buffer_pool_manager.hpp"
#include "catalog/catalog_manager.hpp"

#include <memory>

namespace db {
class ExecutorContext {
public:
	ExecutorContext(std::unique_ptr<CatalogManager> &catalog, std::unique_ptr<BufferPoolManager> &bpm)
	    : catalog_ {catalog}, bpm_ {bpm} {
	}

	~ExecutorContext() = default;

	DISALLOW_COPY_AND_MOVE(ExecutorContext);

	auto GetCatalog() -> std::unique_ptr<CatalogManager> & {
		return catalog_;
	}

	auto GetBufferPoolManager() -> std::unique_ptr<BufferPoolManager> & {
		return bpm_;
	}

private:
	std::unique_ptr<CatalogManager> &catalog_;
	std::unique_ptr<BufferPoolManager> &bpm_;
};
} // namespace db
