#include "catalog/catalog_manager.hpp"

namespace db {
void CatalogManager::Init() {
}

void CatalogManager::CreateTable(const std::string &table_name, const Schema &schema) {

}
void CatalogManager::GeTableInfo(const std::string &table_name) const {

  // std::unordered_map<std::string, std::shared_ptr<TableInfo>> tables_;

  auto table_info = tables_.find(table_name);
}
} // namespace db
