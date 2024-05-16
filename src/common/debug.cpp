#include "common/debug.hpp"
#include <iomanip>
#include <iostream>
namespace db {
void printData(const char *data, std::size_t size) {
  std::cout << "Data: ";
  for (std::size_t i = 0; i < size; ++i) {
    std::cout << std::hex << std::setw(2) << std::setfill('0')
              << (static_cast<int>(data[i]) & 0xff) << " ";
  }
  std::cout << std::dec << std::endl;
}
} // namespace db
