#pragma once

#include "common/typedef.hpp"

#include <type_traits>

namespace db {

class WriteStream {
public:
	// Writes a set amount of data from the specified buffer into the stream and
	// moves the stream forward accordingly
	virtual void WriteData(const_data_ptr_t buffer, idx_t write_size) = 0;

	// // Writes a type into the stream and moves the stream forward sizeof(T) bytes
	// // The type must be a standard layout type
	// template <class T>
	// void Write(T element) {
	// 	static_assert(std::is_standard_layout<T>(), "Write element must be a standard layout data type");
	// 	WriteData(conata_ptr_cast(&element), sizeof(T));
	// }

	virtual ~WriteStream() = default;
};

} // namespace db
