#include "storage/disk_manager.hpp"

#include "catalog/catalog_manager.hpp"
#include "common/config.hpp"
#include "common/fs_utils.hpp"
#include "common/logger.hpp"
#include "storage/file_path_manager.hpp"

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace db {

void DiskManager::AddTableDataIfNotExist(table_oid_t table_id) {
	if (!table_data_files_.contains(table_id)) {
		fs::path table_data_path;
		if (table_id == SYSTEM_CATALOG_ID) {
			table_data_path = FilePathManager::GetInstance().GetSystemCatalogPath();
		} else {
			auto table_name = cm_->GetTableName(table_id);
			table_data_path = FilePathManager::GetInstance().GetTableDataPath(table_name);
		}
		auto data_fs = std::fstream(table_data_path, std::ios::binary | std::ios::in | std::ios::out);
		if (!data_fs.is_open()) {
			data_fs.clear();
			// create a new file
			data_fs.open(table_data_path, std::ios::binary | std::ios::trunc | std::ios::out | std::ios::in);
			if (!data_fs.is_open()) {
				throw IOException("failed to open table data file");
			}
		}
		table_data_files_.emplace(table_id, std::move(data_fs));
	}
}

void DiskManager::WritePage(PageId page_id, const char *page_data) {
	AddTableDataIfNotExist(page_id.table_id_);

	auto &data_fs = table_data_files_.at(page_id.table_id_);
	auto offset = page_id.page_number_ * PAGE_SIZE;
	data_fs.seekp(offset);
	data_fs.write(page_data, PAGE_SIZE);
	if (data_fs.bad()) {
		throw IOException("failed to write to table data file");
	}
	// flush to sync data
	data_fs.flush();
}

void DiskManager::ReadPage(PageId page_id, char *page_data) {
	AddTableDataIfNotExist(page_id.table_id_);

	size_t offset = static_cast<size_t>(page_id.page_number_) * PAGE_SIZE;
	auto data_file_path = FilePathManager::GetInstance().GetTableDataPath(cm_->GetTableName(page_id.table_id_));
	if (offset > GetFileSize(data_file_path)) {
		throw IOException("read page out of file size" + std::to_string(offset) + " " +
		                  std::to_string(GetFileSize(data_file_path)));
	}
	auto &data_fs = table_data_files_.at(page_id.table_id_);
	data_fs.seekp(static_cast<int64_t>(offset));
	data_fs.read(page_data, PAGE_SIZE);
	if (data_fs.bad()) {
		throw IOException("failed to read from table data file");
	}
	// if file ends before reading PAGE_SIZE
	uint32_t gcount = data_fs.gcount();
	if (gcount < PAGE_SIZE) {
		// todo investigate this
		LOG_ERROR("IO read less than a page, read {} rather than {}", gcount, PAGE_SIZE);
		data_fs.clear();
		memset(page_data + gcount, 0, PAGE_SIZE - gcount);
		data_fs.seekp(static_cast<int64_t>(offset));
		data_fs.write(page_data, PAGE_SIZE);
	}
}

void DiskManager::ShutDown() {
	for (auto &[table_id, data_fs] : table_data_files_) {
		data_fs.close();
	}
	for (auto &[table_id, meta_fs] : table_meta_files_) {
		meta_fs.close();
	}
}

DiskManager::~DiskManager() {
	ShutDown();
}

} // namespace db
