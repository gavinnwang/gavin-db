#include "meta/schema.hpp"
#include "common/value.hpp"
#include "storage/table/tuple.hpp"

#include "gtest/gtest.h"
TEST(StorageTest, TupleTest) {
	auto c1 = db::Column("user_id", db::TypeId::INTEGER);
	auto c2 = db::Column("user_name", db::TypeId::VARCHAR, 40);
	auto c3 = db::Column("user_middle_name", db::TypeId::VARCHAR, 40);
	auto schema = db::Schema({c1, c2, c3});
	int32_t int_val = 2;
	std::string str_val = "gavin";
	std::string str_val_2 = "wang";
	auto v1 = db::Value(db::TypeId::INTEGER, int_val);
	auto v2 = db::Value(db::TypeId::VARCHAR, str_val);
	auto v3 = db::Value(db::TypeId::VARCHAR, str_val_2);
	auto tuple = db::Tuple({v1, v2, v3}, schema);
	ASSERT_EQ(tuple.ToString(schema), "(2, gavin, wang)");
}

TEST(StorageTest, TupleTest2) {
	auto c1 = db::Column("user_id", db::TypeId::INTEGER);
	auto c2 = db::Column("user_name", db::TypeId::VARCHAR, 3);
	auto schema = db::Schema({c1, c2});
	int32_t int_val = 2;
	std::string str_val = "hii4";
	auto v1 = db::Value(db::TypeId::INTEGER, int_val);
	auto v2 = db::Value(db::TypeId::VARCHAR, str_val);

	EXPECT_THROW(db::Tuple({v1, v2}, schema), std::runtime_error);
}
