#include "storage/disk_manager.hpp"

#include "common/config.hpp"

#include <cstdlib>
#include <iostream>
#include <sys/stat.h>

namespace db {
DiskManager::DiskManager(const std::string &db_file) : file_name_(db_file) {
	db_io_.open(db_file, std::ios::binary | std::ios::in | std::ios::out);
	if (!db_io_.is_open()) {
		db_io_.clear();
		// create a new file
		db_io_.open(db_file, std::ios::binary | std::ios::trunc | std::ios::out | std::ios::in);
		if (!db_io_.is_open()) {
			std::cout << "failed to open db file";
			exit(0);
		}
	}
}

void DiskManager::WritePage(page_id_t page_id, const char *page_data) {
	size_t offset = page_id * PAGE_SIZE;
	db_io_.seekp(offset);
	db_io_.write(page_data, PAGE_SIZE);
	if (db_io_.bad()) {
		std::cout << "IO error while writing";
		exit(0);
	}
	// flush to sync data
	db_io_.flush();
}

void DiskManager::ReadPage(page_id_t page_id, char *page_data) {
	int offset = page_id * PAGE_SIZE;
	if (offset > GetFileSize(file_name_)) {
		std::cout << "I/O error reading past end of file";
		exit(0);
	} else {
		db_io_.seekp(offset);
		db_io_.read(page_data, PAGE_SIZE);
		if (db_io_.bad()) {
			std::cout << "failed to open db file";
			exit(0);
		}
		// if file ends before reading PAGE_SIZE
		int gcount = db_io_.gcount();
		if (gcount < PAGE_SIZE) {
			std::cout << "io read less than a page, read " << gcount << " rather than " << PAGE_SIZE << std::endl;
			db_io_.clear();
			memset(page_data + gcount, 0, PAGE_SIZE - gcount);
			db_io_.seekp(offset);
			db_io_.write(page_data, PAGE_SIZE);
		}
	}
}

void DiskManager::ShutDown() {
	db_io_.close();
}

DiskManager::~DiskManager() {
	ShutDown();
}

auto DiskManager::GetFileSize(const std::string &file_name) -> int {
	struct stat stat_buf;
	int rc = stat(file_name.c_str(), &stat_buf);
	return rc == 0 ? static_cast<int>(stat_buf.st_size) : -1;
}
} // namespace db
