
#include "storage/serializer/file_stream.hpp"

#include "common/exception.hpp"

#include <cassert>
#include <sstream>
#include <string>

namespace db {

FileStream::FileStream(const std::string &file_path, std::ios::openmode mode) : file_path(file_path), owns_file(true) {
	file.open(file_path, mode);
	if (!file.is_open()) {
		throw Exception("Failed to open file: " + file_path);
	}
	file.clear(); // Clear any error state
	file.seekg(0, std::ios::beg);
	file.seekp(0, std::ios::beg);
}

FileStream::FileStream(std::fstream &&file) : file(std::move(file)), owns_file(false) {
	if (!this->file.is_open()) {
		throw Exception("Provided fstream is not open");
	}
}

FileStream::~FileStream() {
	if (owns_file && file.is_open()) {
		file.close();
	}
}

void FileStream::WriteData(const_data_ptr_t buffer, idx_t write_size) {
	if (!file.write(const_char_ptr_cast(buffer), write_size)) {
		throw Exception("Failed to write data to file");
	}
	file.flush();
}

void FileStream::ReadData(data_ptr_t buffer, idx_t read_size) {
	if (!file.read(reinterpret_cast<char *>(buffer), read_size)) {
		std::ostringstream error_msg;
		error_msg << "Failed to read data from file. ";

		// Check the stream state and append relevant messages
		if (file.eof()) {
			error_msg << "Reason: End of file reached.";
		} else if (file.fail()) {
			error_msg << "Reason: Logical error on i/o operation.";
		} else if (file.bad()) {
			error_msg << "Reason: Read/writing error on i/o operation.";
		} else {
			error_msg << "Reason: Unknown error.";
		}

		throw Exception(error_msg.str());
	}
}

void FileStream::Rewind() {
	file.clear(); // Clear any errors
	file.seekg(0, std::ios::beg);
	file.seekp(0, std::ios::beg);
	assert(file.tellg() == 0);
}

void FileStream::Release() {
	owns_file = false;
}

idx_t FileStream::GetPosition() {
	return static_cast<idx_t>(file.tellg());
}

std::string FileStream::GetFilePath() const {
	return file_path;
}

} // namespace db
