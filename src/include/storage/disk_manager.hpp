#pragma once

#include "common/config.hpp"
#include "common/macros.hpp"

#include <fstream>
#include <string>

namespace db {
class DiskManager {
public:
	explicit DiskManager(const std::string &db_file);
	void ShutDown();
	void WritePage(page_id_t page_id, const char *page_data);
	void ReadPage(page_id_t page_id, char *page_data);
	~DiskManager();
	DISALLOW_COPY(DiskManager);

private:
	auto GetFileSize(const std::string &file_name) -> int;
	std::string file_name_;
	std::fstream db_io_;
};
} // namespace db
