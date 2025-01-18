#include "meta/column.hpp"
#include "meta/schema.hpp"
#include "common/logger.hpp"
#include "common/type.hpp"
#include "common/value.hpp"
#include "storage/serializer/binary_deserializer.hpp"
#include "storage/serializer/binary_serializer.hpp"
#include "storage/serializer/deserializer.hpp"
#include "storage/serializer/file_stream.hpp"
#include "storage/table/table_meta.hpp"

#include "gtest/gtest.h"
#include <cstdint>
#include <memory>
#include <vector>
namespace db {

struct Bar {
	uint32_t b_;
	std::vector<std::string> vec_;
	void Serialize(Serializer &serializer) const {
		serializer.WriteProperty<uint32_t>(1, "b", b_);
		serializer.WritePropertyWithDefault(2, "vec", vec_, std::vector<std::string>());
	}

	static std::unique_ptr<Bar> Deserialize(Deserializer &deserializer) {
		auto result = std::make_unique<Bar>();
		deserializer.ReadProperty<uint32_t>(1, "b", result->b_);
		deserializer.ReadPropertyWithDefault(2, "vec", result->vec_, std::vector<std::string>());
		return result;
	}
};

struct Foo {
	int32_t a_;
	std::unique_ptr<Bar> bar_;
	int32_t c_;
	TypeId type_;
	std::vector<std::unique_ptr<Bar>> bars_;
	std::unordered_map<int32_t, std::unique_ptr<Bar>> bar_map_;
	Value value_ {TypeId::INTEGER, 10};

	void Serialize(Serializer &serializer) const {
		serializer.WriteProperty<int32_t>(1, "a", a_);
		serializer.WritePropertyWithDefault<std::unique_ptr<Bar>>(2, "bar", bar_, std::unique_ptr<Bar>());
		serializer.WriteProperty<int32_t>(3, "c", c_);
		serializer.WriteProperty<TypeId>(4, "type", type_);
		serializer.WriteProperty<std::vector<std::unique_ptr<Bar>>>(5, "bars", bars_);
		serializer.WriteProperty<Value>(6, "value", value_);
		serializer.WriteProperty(7, "bar_map", bar_map_);
	}

	static std::unique_ptr<Foo> Deserialize(Deserializer &deserializer) {
		auto result = std::make_unique<Foo>();
		deserializer.ReadProperty<int32_t>(1, "a", result->a_);
		deserializer.ReadPropertyWithDefault<std::unique_ptr<Bar>>(2, "bar", result->bar_, std::unique_ptr<Bar>());
		deserializer.ReadProperty<int32_t>(3, "c", result->c_);
		deserializer.ReadProperty<TypeId>(4, "type", result->type_);
		deserializer.ReadProperty<std::vector<std::unique_ptr<Bar>>>(5, "bars", result->bars_);
		deserializer.ReadProperty<Value>(6, "value", result->value_);
		deserializer.ReadProperty(7, "bar_map", result->bar_map_);

		return result;
	}
};

TEST(StorageTest, SerializerTest) {
	Foo foo_in;
	foo_in.a_ = 42;
	foo_in.bar_ = std::make_unique<Bar>();
	foo_in.bar_->b_ = 43;
	std::string value_str = "hello";
	foo_in.value_ = Value(TypeId::VARCHAR, value_str);
	// lambda to create a bar
	auto create_bar = [](uint32_t b) {
		Bar bar;
		bar.b_ = b;
		return bar;
	};
	foo_in.bars_.emplace_back(std::make_unique<Bar>(create_bar(44)));
	foo_in.bars_.emplace_back(std::make_unique<Bar>(create_bar(45)));
	foo_in.bars_.emplace_back(std::make_unique<Bar>(create_bar(46)));
	foo_in.bar_map_[1] = std::make_unique<Bar>(create_bar(47));
	foo_in.bar_map_[2] = std::make_unique<Bar>(create_bar(48));
	foo_in.bar_map_[120338] = std::make_unique<Bar>(create_bar(120338));
	std::vector<std::string> vec_str = {"a", "a", "a", "b", "c", "d", "a", "a", "a"};
	foo_in.bar_->vec_ = vec_str;
	foo_in.c_ = 44;
	foo_in.type_ = TypeId::VARCHAR;
	FileStream stream("test_file", std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

	BinarySerializer::Serialize(foo_in, stream, false);
	auto pos1 = stream.GetPosition();
	stream.Print();
	stream.Rewind();

	auto foo_out_ptr = BinaryDeserializer::Deserialize<Foo>(stream);
	auto &foo_out = *foo_out_ptr;
	EXPECT_EQ(foo_in.a_, foo_out.a_);
	EXPECT_EQ(foo_in.bar_->b_, foo_out.bar_->b_);
	EXPECT_EQ(foo_in.c_, foo_out.c_);
	EXPECT_EQ(foo_in.type_, foo_out.type_);
	for (size_t i = 0; i < vec_str.size(); i++) {
		EXPECT_EQ(vec_str[i], foo_out.bar_->vec_[i]);
	}
	for (size_t i = 0; i < foo_in.bars_.size(); i++) {
		EXPECT_EQ(foo_in.bars_[i]->b_, foo_out.bars_[i]->b_);
	}
	EXPECT_EQ(value_str, foo_out.value_.ToString());
	for (const auto &[key, value] : foo_in.bar_map_) {
		EXPECT_EQ(value->b_, foo_out.bar_map_[key]->b_);
	}

	foo_in.bar_ = nullptr;

	stream.Rewind();

	BinarySerializer::Serialize(foo_in, stream, false);
	auto pos2 = stream.GetPosition();
	stream.Print();
	stream.Rewind();

	foo_out_ptr = BinaryDeserializer::Deserialize<Foo>(stream);
	auto &foo_out2 = *foo_out_ptr;
	EXPECT_EQ(foo_in.a_, foo_out2.a_);
	EXPECT_TRUE(foo_in.bar_ == nullptr && foo_out2.bar_ == nullptr);
	EXPECT_EQ(foo_in.c_, foo_out2.c_);
	EXPECT_EQ(foo_in.type_, foo_out2.type_);
	EXPECT_EQ(value_str, foo_out2.value_.ToString());

	LOG_DEBUG("pos1: %du pos2: %du", static_cast<uint32_t>(pos1), static_cast<uint32_t>(pos2));
  // should not write the default value
	EXPECT_TRUE(pos1 > pos2);
}

TEST(StorageTest, SerializeValueTest) {
	auto c1 = Column("user_id", db::TypeId::INTEGER);
	auto c2 = Column("user_name", db::TypeId::VARCHAR, 256);
	auto c3 = Column("user_location", db::TypeId::VARCHAR, 256);
	auto c4 = Column("user_age", db::TypeId::INTEGER);
	auto c5 = Column("user_preference", db::TypeId::VARCHAR, 64);
	auto c6 = Column("user_email", db::TypeId::VARCHAR, 30);
	auto c7 = Column("user_last_active", db::TypeId::TIMESTAMP);
	auto c8 = Column("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", db::TypeId::INTEGER);
	auto schema = Schema({c1, c2, c3, c4, c5, c6, c7, c8});

	auto table_meta = TableMeta {schema, "table_name", 1};
	FileStream stream("schema_test_file", std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
	BinarySerializer::Serialize(table_meta, stream, false);
	stream.Print();
	stream.Rewind();

	auto out = BinaryDeserializer::Deserialize<TableMeta>(stream);
	LOG_DEBUG("schema: %s", out->schema_.ToString().c_str());
	LOG_DEBUG("name: %s", out->name_.c_str());
}
} // namespace db
