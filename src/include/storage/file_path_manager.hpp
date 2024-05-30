#pragma once

#include "common/config.hpp"

#include <filesystem>
#include <string>
namespace db {
namespace fs = std::filesystem;
// singleton
class FilePathManager {

public:
	FilePathManager(const FilePathManager &) = delete;
	FilePathManager(FilePathManager &&) = delete;
	FilePathManager &operator=(const FilePathManager &) = delete;
	FilePathManager &operator=(FilePathManager &&) = delete;

	static FilePathManager &GetInstance() {
		static FilePathManager instance {};
		instance.SetDatabaseName(DEFAULT_DB_NAME);
		return instance;
	}

	void SetDatabaseName(const std::string &db_name) {
		db_path_ = db_name;
	}

	fs::path GetTableMetaPath(const std::string &table_name) {
		fs::path table_path = table_name;
		fs::path meta_path = db_path_ / table_path / "meta";
		return meta_path;
	}

	fs::path GetTableDataPath(const std::string &table_name) {
		fs::path table_path = table_name;
		fs::path data_path = db_path_ / table_path / "data";
		return data_path;
	}

private:
	fs::path db_path_;
	FilePathManager() = default;
	~FilePathManager() = default;
};
} // namespace db
