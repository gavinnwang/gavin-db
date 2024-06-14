#include "storage/serializer/memory_stream.hpp"

#include "common/exception.hpp"

#include <cstdlib>
#include <cstring>
namespace db {

MemoryStream::MemoryStream(idx_t capacity_)
    : position_(0), capacity_(capacity_), owns_data_(true), data_(static_cast<data_ptr_t>(malloc(capacity_))) {
}

MemoryStream::MemoryStream(data_ptr_t buffer, idx_t capacity)
    : position_(0), capacity_(capacity), owns_data_(false), data_(buffer) {
}

MemoryStream::~MemoryStream() {
	if (owns_data_) {
		free(data_);
	}
}

void MemoryStream::WriteData(const_data_ptr_t buffer, idx_t write_size) {
	while (position_ + write_size > capacity_) {
		if (owns_data_) {
			capacity_ *= 2;
			data_ = static_cast<data_ptr_t>(realloc(data_, capacity_));
		} else {
			throw Exception("Failed to serialize: not enough space in buffer to fulfill write request");
		}
	}

	memcpy(data_ + position_, buffer, write_size);
	position_ += write_size;
}

void MemoryStream::ReadData(data_ptr_t buffer, idx_t read_size) {
	if (position_ + read_size > capacity_) {
		throw Exception("Failed to deserialize: not enough data_ in buffer to fulfill read request");
	}
	memcpy(buffer, data_ + position_, read_size);
	position_ += read_size;
}

void MemoryStream::Rewind() {
	position_ = 0;
}

void MemoryStream::Release() {
	owns_data_ = false;
}

data_ptr_t MemoryStream::GetData() const {
	return data_;
}

idx_t MemoryStream::GetPosition() const {
	return position_;
}

idx_t MemoryStream::GetCapacity() const {
	return capacity_;
}

} // namespace db
