#pragma once

#include "common/exception.hpp"
#include "common/logger.hpp"

#include <filesystem>
#include <fstream>
#include <sys/stat.h>
namespace db {
namespace fs = std::filesystem;

inline void DeletePathIfExists(const std::filesystem::path &path) {
	try {
		if (std::filesystem::exists(path)) {
			std::filesystem::remove_all(path);
		}
	} catch (const std::filesystem::filesystem_error &e) {
		throw Exception(std::string("Failed to delete path: ") + e.what());
	}
}

inline size_t GetFileSize(const std::filesystem::path &file_path) {
	struct stat stat_buf;
	int stat_code = stat(file_path.c_str(), &stat_buf);
	return stat_code == 0 ? static_cast<size_t>(stat_buf.st_size) : -1;
}

inline void CreateFolderIfNotExists(const std::filesystem::path &folder_path) {
	if (std::filesystem::exists(folder_path)) {
		return;
	}
	std::error_code error_code;
	std::filesystem::create_directories(folder_path, error_code);
	if (error_code) {
		throw IOException("failed to create directories: " + folder_path.string() + error_code.message());
	}
}
inline void CreateFileIfNotExists(const std::filesystem::path &file_path) {
	fs::path directory_path = file_path.parent_path();
	if (!fs::exists(directory_path)) {
		CreateFolderIfNotExists(directory_path);
	}
	if (fs::exists(file_path)) {
		return;
	}
	LOG_TRACE("Creating file as it doesn't exist {}", file_path.string());

	std::ofstream file(file_path);
	if (!file.is_open()) {
		throw IOException("failed to create file: " + file_path.string());
	}

	file.close();
}

} // namespace db
