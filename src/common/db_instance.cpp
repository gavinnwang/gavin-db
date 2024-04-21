#include "common/db_instance.hpp"
#include "catalog/column.hpp"
#include "catalog/schema.hpp"
#include <iostream>

namespace db {

DBInstance::DBInstance(const std::string &db_file_name) {
    std::cout << db_file_name << std::endl;
    auto col= Column("c1", TypeId::INTEGER);
    auto s = Schema({col});
};
}
