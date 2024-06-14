#pragma once

#include "buffer/buffer_pool_manager.hpp"
namespace db {

class ExecutionEngine {
public:
	DISALLOW_COPY_AND_MOVE(ExecutionEngine);

private:
	const std::shared_ptr<BufferPoolManager> &buffer_pool_manager_;
};

} // namespace db
