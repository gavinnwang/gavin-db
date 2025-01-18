#pragma once

#include "common/macros.hpp"
#include "common/page_id.hpp"
#include "common/typedef.hpp"

#include <fstream>
#include <memory>
#include <unordered_map>

namespace db {
class Catalog; // forward declaration

class DiskManager {
public:
	explicit DiskManager(std::unique_ptr<Catalog> &catalog) : cm_(catalog) {
	}
	void ShutDown();
	void WritePage(PageId page_id, const char *page_data);
	void ReadPage(PageId page_id, char *page_data);
	~DiskManager();
	DISALLOW_COPY(DiskManager);

private:
	void AddTableDataIfNotExist(table_oid_t table_id);
	std::unique_ptr<Catalog> &cm_;
	std::unordered_map<table_oid_t, std::fstream> table_data_files_;
	std::unordered_map<table_oid_t, std::fstream> table_meta_files_;
};
} // namespace db
