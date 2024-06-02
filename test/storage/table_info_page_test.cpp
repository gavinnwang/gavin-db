#include "buffer/buffer_pool_manager.hpp"
#include "common/exception.hpp"
#include "common/fs_utils.hpp"
#include "common/typedef.hpp"
#include "storage/file_path_manager.hpp"
#include "storage/table_heap.hpp"
#include "storage/table_info_page.hpp"

#include "gtest/gtest.h"
#include <stdexcept>


TEST(StorageTest, SimpleTableInfoPageTest) {
	db::DeletePathIfExists(db::FilePathManager::GetInstance().GetDatabaseRootPath());
	const size_t buffer_pool_size = 10;
	auto cm = std::make_shared<db::CatalogManager>();
	auto dm = std::make_shared<db::DiskManager>(cm);
	db::table_oid_t table_oid = 0;
	auto bpm = std::make_shared<db::BufferPoolManager>(buffer_pool_size, std::move(dm), cm);
	auto c1 = db::Column("user_id", db::TypeId::INTEGER);
	auto c2 = db::Column("user_name", db::TypeId::VARCHAR, 256);
	auto schema = db::Schema({c1, c2});
	auto table_name = "usr";
	cm->CreateTable(table_name, schema);
}

TEST(StorageTest, DuplicateTableNameTest) {
	db::DeletePathIfExists(db::FilePathManager::GetInstance().GetDatabaseRootPath());
	const size_t buffer_pool_size = 10;
	auto cm = std::make_shared<db::CatalogManager>();
	auto dm = std::make_shared<db::DiskManager>(cm);
	db::table_oid_t table_oid = 0;
	auto bpm = std::make_shared<db::BufferPoolManager>(buffer_pool_size, std::move(dm), cm);
	auto c1 = db::Column("user_id", db::TypeId::INTEGER);
	auto c2 = db::Column("user_name", db::TypeId::VARCHAR, 256);
	auto schema = db::Schema({c1, c2});
	auto table_name = "usr";
	cm->CreateTable(table_name, schema);
	EXPECT_THROW(cm->CreateTable(table_name, schema), std::runtime_error);
}
