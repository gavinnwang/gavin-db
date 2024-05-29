#pragma once

#include "storage/serializer/deserializer.hpp"
#include "storage/serializer/read_stream.hpp"

#include <cassert>
namespace db {

class BinaryDeserializer : public Deserializer {

private:
	ReadStream &stream;
	idx_t nesting_level = 0;

	// bool has_buffered_field = false;
	// field_id_t buffered_field = 0;

	explicit BinaryDeserializer(ReadStream &stream) : stream(stream) {
	}

	template <class T>
	std::unique_ptr<T> Deserialize() {
		OnObjectBegin();
		auto result = T::Deserialize(*this);
		OnObjectEnd();
		assert(nesting_level == 0); // make sure we are at the root level
		return result;
	}
	template <class T>
	static std::unique_ptr<T> Deserialize(ReadStream &stream) {
		BinaryDeserializer deserializer(stream);
		return Deserialize<T>();
		// return deserializer.template Deserialize<T>();
	}

	ReadStream &GetStream() {
		return stream;
	}

private:
	field_id_t PeekField() {
		if (!has_buffered_field) {
			buffered_field = ReadPrimitive<field_id_t>();
			has_buffered_field = true;
		}
		return buffered_field;
	}
	void ConsumeField() {
		if (!has_buffered_field) {
			buffered_field = ReadPrimitive<field_id_t>();
		} else {
			has_buffered_field = false;
		}
	}
};
} // namespace db
