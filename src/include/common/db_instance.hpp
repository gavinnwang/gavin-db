#pragma once

#include "buffer/buffer_pool_manager.hpp"

#include <memory>
#include <string>
namespace db {

class DB {
public:
	explicit DB(const std::string &db_file_name);
	~DB();

private:
	std::shared_ptr<BufferPoolManager> bpm_;
	std::shared_ptr<CatalogManager> catalog_;
};

}; // namespace db
