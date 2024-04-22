#include <common/db_instance.hpp>
#include <iostream>

int main() {
  db::DBInstance("test.db");
  std::cout << "Welcome to the gavin db!" << std::endl << std::endl;
  while(true) {
    std::string query;

  }
}

