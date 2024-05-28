#include "storage/serializer/binary_serializer.hpp"
#include "storage/serializer/memory_stream.hpp"

#include "gtest/gtest.h"
namespace db {

struct Bar {
	uint32_t b;
	void Serialize(Serializer &serializer) const {
		serializer.WriteProperty<uint32_t>(1, "b", b);
	}

	// static unique_ptr<Bar> Deserialize(Deserializer &deserializer) {
	// 	auto result = make_uniq<Bar>();
	// 	deserializer.ReadProperty<uint32_t>(1, "b", result->b);
	// 	return result;
	// }
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

	// static std::unique_ptr<Foo> Deserialize(Deserializer &deserializer) {
	// 	auto result = make_uniq<Foo>();
	// 	deserializer.ReadProperty<int32_t>(1, "a", result->a);
	// 	deserializer.ReadPropertyWithDefault<unique_ptr<Bar>>(2, "bar", result->bar, unique_ptr<Bar>());
	// 	deserializer.ReadProperty<int32_t>(3, "c", result->c);
	// 	return result;
	// }
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
	stream.Print();
	stream.Rewind();
}
} // namespace db
