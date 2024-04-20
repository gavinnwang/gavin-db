#include "common/db_instance.hpp"
#include <iostream>

namespace db {

DBInstance::DBInstance(const std::string &db_file_name) {
    std::cout << db_file_name << std::endl;
}
}
