
#include <common/db_instance.hpp>
#include <iostream>

int main() {
  std::cout << "Starting shell" << std::endl;
  db::DBInstance("hi");
}
