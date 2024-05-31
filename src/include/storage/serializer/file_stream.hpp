#pragma once

#include "storage/serializer/read_stream.hpp"
#include "storage/serializer/write_stream.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace db {
class FileStream : public WriteStream, public ReadStream {
private:
	std::fstream file;
	std::string file_path;
	bool owns_file;

public:
	// Create a new FileStream with the specified file path. The stream will own the file and handle its closing.
	explicit FileStream(const std::string &file_path,
	                    std::ios::openmode mode = std::ios::in | std::ios::out | std::ios::binary);

	// Create a new FileStream over an existing file stream.
	// The stream will not take ownership of the file, will not attempt to close it.
	explicit FileStream(std::fstream &&file);

	~FileStream() override;

	// Write data to the stream.
	// Throws if the write fails.
	void WriteData(const_data_ptr_t buffer, idx_t write_size) override;

	// Read data from the stream.
	// Throws if the read fails.
	void ReadData(data_ptr_t buffer, idx_t read_size) override;

	// Rewind the stream to the start.
	void Rewind();

	// Release ownership of the file stream.
	void Release();

	// Get the current position in the stream.
	idx_t GetPosition();

	// Get the file path.
	std::string GetFilePath() const;

	// Debugging function to print the file content in hexadecimal format.
	void Print() {
		std::streampos current_pos = file.tellg();
		std::cout << "Printing file with size " << current_pos << std::endl;

		// Move to the beginning of the file
		file.seekg(0, std::ios::beg);

		// Read up to the current position into a buffer
		std::vector<char> buffer(current_pos);
		file.read(buffer.data(), current_pos);

		// Print each byte in hexadecimal format
		for (const auto &byte : buffer) {
			printf("%02x ", static_cast<unsigned char>(byte));
		}
		printf("\n");

		// Restore the file position
		file.seekg(current_pos);
	}
};
} // namespace db
