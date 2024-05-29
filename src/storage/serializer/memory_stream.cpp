#include "storage/serializer/memory_stream.hpp"

#include "common/exception.hpp"

#include <cstdlib>
namespace db {

MemoryStream::MemoryStream(idx_t capacity)
    : position(0), capacity(capacity), owns_data(true), data(static_cast<data_ptr_t>(malloc(capacity))) {
}

MemoryStream::MemoryStream(data_ptr_t buffer, idx_t capacity)
    : position(0), capacity(capacity), owns_data(false), data(buffer) {
}

MemoryStream::~MemoryStream() {
	if (owns_data) {
		free(data);
	}
}

void MemoryStream::WriteData(const_data_ptr_t source, idx_t write_size) {
	while (position + write_size > capacity) {
		if (owns_data) {
			capacity *= 2;
			data = static_cast<data_ptr_t>(realloc(data, capacity));
		} else {
			throw Exception("Failed to serialize: not enough space in buffer to fulfill write request");
		}
	}

	memcpy(data + position, source, write_size);
	position += write_size;
}

void MemoryStream::Rewind() {
	position = 0;
}

void MemoryStream::Release() {
	owns_data = false;
}

data_ptr_t MemoryStream::GetData() const {
	return data;
}

idx_t MemoryStream::GetPosition() const {
	return position;
}

idx_t MemoryStream::GetCapacity() const {
	return capacity;
}

} // namespace db