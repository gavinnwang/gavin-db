
#include "storage/serializer/file_stream.hpp"

#include "common/exception.hpp"

#include <cassert>
#include <sstream>
#include <string>

namespace db {

FileStream::FileStream(const std::filesystem::path &file_path, std::ios::openmode mode)
    : file_path_(file_path), owns_file_(true) {
	file_.open(file_path, mode);
	if (!file_.is_open()) {
		throw Exception("Failed to open file: " + file_path.string());
	}
	file_.clear(); // Clear any error state
	file_.seekg(0, std::ios::beg);
	file_.seekp(0, std::ios::beg);
}

FileStream::FileStream(std::fstream &&file) : file_(std::move(file)), owns_file_(false) {
	if (!this->file_.is_open()) {
		throw Exception("Provided fstream is not open");
	}
}

FileStream::~FileStream() {
	if (owns_file_ && file_.is_open()) {
		file_.close();
	}
}

void FileStream::WriteData(const_data_ptr_t buffer, idx_t write_size) {
	if (!file_.write(const_char_ptr_cast(buffer), write_size)) {
		throw Exception("Failed to write data to file");
	}
	file_.flush();
}

void FileStream::ReadData(data_ptr_t buffer, idx_t read_size) {
	if (!file_.read(reinterpret_cast<char *>(buffer), read_size)) {
		std::ostringstream error_msg;
		error_msg << "Failed to read data from file. ";

		// Check the stream state and append relevant messages
		if (file_.eof()) {
			error_msg << "Reason: End of file reached.";
		} else if (file_.fail()) {
			error_msg << "Reason: Logical error on i/o operation.";
		} else if (file_.bad()) {
			error_msg << "Reason: Read/writing error on i/o operation.";
		} else {
			error_msg << "Reason: Unknown error.";
		}

		throw Exception(error_msg.str());
	}
}

void FileStream::Rewind() {
	file_.clear(); // Clear any errors
	file_.seekg(0, std::ios::beg);
	file_.seekp(0, std::ios::beg);
	assert(file_.tellg() == 0);
}

void FileStream::Release() {
	owns_file_ = false;
}

idx_t FileStream::GetPosition() {
	return static_cast<idx_t>(file_.tellg());
}

std::string FileStream::GetFilePath() const {
	return file_path_;
}

} // namespace db
