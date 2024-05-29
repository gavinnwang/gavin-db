#include "storage/serializer/binary_deserializer.hpp"
#include "storage/serializer/binary_serializer.hpp"
#include "storage/serializer/deserializer.hpp"
#include "storage/serializer/memory_stream.hpp"

#include "gtest/gtest.h"
namespace db {

struct Bar {
	uint32_t b;
	void Serialize(Serializer &serializer) const {
		serializer.WriteProperty<uint32_t>(1, "b", b);
	}

	static std::unique_ptr<Bar> Deserialize(Deserializer &deserializer) {
		auto result = std::make_unique<Bar>();
		deserializer.ReadProperty<uint32_t>(1, "b", result->b);
		return result;
	}
};

struct Foo {
	int32_t a;
	std::unique_ptr<Bar> bar;
	int32_t c;

	void Serialize(Serializer &serializer) const {
		serializer.WriteProperty<int32_t>(1, "a", a);
		serializer.WritePropertyWithDefault<std::unique_ptr<Bar>>(2, "bar", bar, std::unique_ptr<Bar>());
		serializer.WriteProperty<int32_t>(3, "c", c);
	}

	static std::unique_ptr<Foo> Deserialize(Deserializer &deserializer) {
		auto result = std::make_unique<Foo>();
		deserializer.ReadProperty<int32_t>(1, "a", result->a);
		deserializer.ReadPropertyWithDefault<std::unique_ptr<Bar>>(2, "bar", result->bar, std::unique_ptr<Bar>());
		deserializer.ReadProperty<int32_t>(3, "c", result->c);
		return result;
	}
};

TEST(StorageTest, SerializerTest) {
	Foo foo_in;
	foo_in.a = 42;
	foo_in.bar = std::make_unique<Bar>();
	foo_in.bar->b = 43;
	foo_in.c = 44;
	MemoryStream stream;

	BinarySerializer::Serialize(foo_in, stream, false);
	auto pos1 = stream.GetPosition();
	// stream.Print();
	stream.Rewind();

	auto foo_out_ptr = BinaryDeserializer::Deserialize<Foo>(stream);
	auto &foo_out = *foo_out_ptr.get();
	// REQUIRE(foo_in.a == foo_out.a);
	// REQUIRE(foo_in.bar->b == foo_out.bar->b);
	// REQUIRE(foo_in.c == foo_out.c);
	EXPECT_EQ(foo_in.a, foo_out.a);
	EXPECT_EQ(foo_in.bar->b, foo_out.bar->b);
	EXPECT_EQ(foo_in.c, foo_out.c);

	foo_in.bar = nullptr;

	stream.Rewind();

	BinarySerializer::Serialize(foo_in, stream, false);
	auto pos2 = stream.GetPosition();
	stream.Rewind();

	foo_out_ptr = BinaryDeserializer::Deserialize<Foo>(stream);
	auto &foo_out2 = *foo_out_ptr.get();
	EXPECT_EQ(foo_in.a, foo_out2.a);
	EXPECT_TRUE(foo_in.bar == nullptr && foo_out2.bar == nullptr);
	EXPECT_EQ(foo_in.c, foo_out2.c);

	// should not write the default value
	EXPECT_TRUE(pos1 > pos2);
}
} // namespace db
