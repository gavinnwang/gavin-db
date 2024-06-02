#include "catalog/column.hpp"
#include "catalog/schema.hpp"
#include "common/type.hpp"
#include "common/value.hpp"
#include "storage/serializer/binary_deserializer.hpp"
#include "storage/serializer/binary_serializer.hpp"
#include "storage/serializer/deserializer.hpp"
#include "storage/serializer/file_stream.hpp"
#include "storage/table_info_page.hpp"

#include "gtest/gtest.h"
#include <memory>
#include <vector>
namespace db {

struct Bar {
	uint32_t b;
	std::vector<std::string> vec;
	void Serialize(Serializer &serializer) const {
		serializer.WriteProperty<uint32_t>(1, "b", b);
		serializer.WritePropertyWithDefault(2, "vec", vec, std::vector<std::string>());
	}

	static std::unique_ptr<Bar> Deserialize(Deserializer &deserializer) {
		auto result = std::make_unique<Bar>();
		deserializer.ReadProperty<uint32_t>(1, "b", result->b);
		deserializer.ReadPropertyWithDefault(2, "vec", result->vec, std::vector<std::string>());
		return result;
	}
};

struct Foo {
	int32_t a;
	std::unique_ptr<Bar> bar;
	int32_t c;
	TypeId type;
	std::vector<std::unique_ptr<Bar>> bars;
	std::unordered_map<int32_t, std::unique_ptr<Bar>> bar_map;
	Value value {TypeId::INTEGER, 10};

	void Serialize(Serializer &serializer) const {
		serializer.WriteProperty<int32_t>(1, "a", a);
		serializer.WritePropertyWithDefault<std::unique_ptr<Bar>>(2, "bar", bar, std::unique_ptr<Bar>());
		serializer.WriteProperty<int32_t>(3, "c", c);
		serializer.WriteProperty<TypeId>(4, "type", type);
		serializer.WriteProperty<std::vector<std::unique_ptr<Bar>>>(5, "bars", bars);
		serializer.WriteProperty<Value>(6, "value", value);
		serializer.WriteProperty(7, "bar_map", bar_map);
	}

	static std::unique_ptr<Foo> Deserialize(Deserializer &deserializer) {
		auto result = std::make_unique<Foo>();
		deserializer.ReadProperty<int32_t>(1, "a", result->a);
		deserializer.ReadPropertyWithDefault<std::unique_ptr<Bar>>(2, "bar", result->bar, std::unique_ptr<Bar>());
		deserializer.ReadProperty<int32_t>(3, "c", result->c);
		deserializer.ReadProperty<TypeId>(4, "type", result->type);
		deserializer.ReadProperty<std::vector<std::unique_ptr<Bar>>>(5, "bars", result->bars);
		deserializer.ReadProperty<Value>(6, "value", result->value);
		deserializer.ReadProperty(7, "bar_map", result->bar_map);

		return result;
	}
};

TEST(StorageTest, SerializerTest) {
	Foo foo_in;
	foo_in.a = 42;
	foo_in.bar = std::make_unique<Bar>();
	foo_in.bar->b = 43;
	auto value_str = "hello";
	foo_in.value = Value(TypeId::VARCHAR, value_str);
	// lambda to create a bar
	auto create_bar = [](uint32_t b) {
		Bar bar;
		bar.b = b;
		return bar;
	};
	foo_in.bars.emplace_back(std::make_unique<Bar>(create_bar(44)));
	foo_in.bars.emplace_back(std::make_unique<Bar>(create_bar(45)));
	foo_in.bars.emplace_back(std::make_unique<Bar>(create_bar(46)));
	foo_in.bar_map[1] = std::make_unique<Bar>(create_bar(47));
	foo_in.bar_map[2] = std::make_unique<Bar>(create_bar(48));
	foo_in.bar_map[120338] = std::make_unique<Bar>(create_bar(120338));
	std::vector<std::string> vec_str = {"a", "a", "a", "b", "c", "d", "a", "a", "a"};
	foo_in.bar->vec = vec_str;
	foo_in.c = 44;
	foo_in.type = TypeId::VARCHAR;
	FileStream stream("test_file", std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

	BinarySerializer::Serialize(foo_in, stream, false);
	auto pos1 = stream.GetPosition();
	stream.Print();
	stream.Rewind();

	auto foo_out_ptr = BinaryDeserializer::Deserialize<Foo>(stream);
	auto &foo_out = *foo_out_ptr.get();
	EXPECT_EQ(foo_in.a, foo_out.a);
	EXPECT_EQ(foo_in.bar->b, foo_out.bar->b);
	EXPECT_EQ(foo_in.c, foo_out.c);
	EXPECT_EQ(foo_in.type, foo_out.type);
	for (size_t i = 0; i < vec_str.size(); i++) {
		EXPECT_EQ(vec_str[i], foo_out.bar->vec[i]);
	}
	for (size_t i = 0; i < foo_in.bars.size(); i++) {
		EXPECT_EQ(foo_in.bars[i]->b, foo_out.bars[i]->b);
	}
	EXPECT_EQ(value_str, foo_out.value.ToString());
	// std::cout << std::endl;
	for (const auto &[key, value] : foo_in.bar_map) {
		// std::cout << key << " " << value->b << std::endl;
		EXPECT_EQ(value->b, foo_out.bar_map[key]->b);
	}
	// std::cout << std::endl;

	foo_in.bar = nullptr;

	stream.Rewind();

	BinarySerializer::Serialize(foo_in, stream, false);
	auto pos2 = stream.GetPosition();
	stream.Print();
	stream.Rewind();

	foo_out_ptr = BinaryDeserializer::Deserialize<Foo>(stream);
	auto &foo_out2 = *foo_out_ptr.get();
	EXPECT_EQ(foo_in.a, foo_out2.a);
	EXPECT_TRUE(foo_in.bar == nullptr && foo_out2.bar == nullptr);
	EXPECT_EQ(foo_in.c, foo_out2.c);
	EXPECT_EQ(foo_in.type, foo_out2.type);
	EXPECT_EQ(value_str, foo_out2.value.ToString());
	std::cout << pos1 << " " << pos2 << std::endl;
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

	auto table_info = TableInfo {schema, "table_name", 1};
	FileStream stream("schema_test_file", std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
	BinarySerializer::Serialize(table_info, stream, false);
	stream.Print();
	stream.Rewind();

	auto out = BinaryDeserializer::Deserialize<TableInfo>(stream);
	std::cout << out->schema_.ToString() << std::endl;
	std::cout << out->name_;
}
} // namespace db
