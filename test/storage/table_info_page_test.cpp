#include "buffer/buffer_pool_manager.hpp"
#include "catalog/catalog_manager.hpp"
#include "common/fs_utils.hpp"
#include "common/typedef.hpp"
#include "storage/file_path_manager.hpp"
#include "storage/table/table_heap.hpp"

#include "gtest/gtest.h"
#include <stdexcept>

TEST(StorageTest, SimpleTableMetaPageTest) {
	db::DeletePathIfExists(db::FilePathManager::GetInstance().GetDatabaseRootPath());
	const size_t buffer_pool_size = 10;
	auto cm = std::make_unique<db::CatalogManager>();
	auto dm = std::make_shared<db::DiskManager>(cm);
	auto bpm = std::make_unique<db::BufferPoolManager>(buffer_pool_size, std::move(dm));
	auto c1 = db::Column("user_id", db::TypeId::INTEGER);
	auto c2 = db::Column("user_name", db::TypeId::VARCHAR, 256);
	auto schema = db::Schema({c1, c2});
	const auto *table_name = "user";
	cm->CreateTable(table_name, schema);
}

TEST(StorageTest, DuplicateTableNameTest) {
	db::DeletePathIfExists(db::FilePathManager::GetInstance().GetDatabaseRootPath());
	const size_t buffer_pool_size = 10;
	auto cm = std::make_unique<db::CatalogManager>();
	auto dm = std::make_shared<db::DiskManager>(cm);
	auto bpm = std::make_unique<db::BufferPoolManager>(buffer_pool_size, std::move(dm));
	auto c1 = db::Column("user_id", db::TypeId::INTEGER);
	auto c2 = db::Column("user_name", db::TypeId::VARCHAR, 256);
	auto schema = db::Schema({c1, c2});
	const auto *table_name = "user";
	cm->CreateTable(table_name, schema);
	EXPECT_TRUE(!cm->CreateTable(table_name, schema).has_value());
}
