#pragma once

#include "buffer/buffer_pool_manager.hpp"
#include "catalog/catalog_manager.hpp"

#include <memory>

namespace db {
class ExecutorContext {
public:
	ExecutorContext(std::shared_ptr<CatalogManager> &catalog, std::shared_ptr<BufferPoolManager> &bpm)
	    : catalog_ {catalog}, bpm_ {bpm} {
	}

	~ExecutorContext() = default;

	DISALLOW_COPY_AND_MOVE(ExecutorContext);

	auto GetCatalog() -> std::shared_ptr<CatalogManager> & {
		return catalog_;
	}

	auto GetBufferPoolManager() -> std::shared_ptr<BufferPoolManager> & {
		return bpm_;
	}

private:
	std::shared_ptr<CatalogManager> &catalog_;
	std::shared_ptr<BufferPoolManager> &bpm_;
};
} // namespace db
