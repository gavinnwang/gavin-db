#include "catalog/catalog.hpp"
#include "storage/catalog_page.hpp"
namespace db {
void Catalog::Init() {
  auto catalog_page = bpm_->FetchPageWrite(0).AsMut<CatalogPage>();
}

void Catalog::CreateTable(const std::string &table_name, const Schema &schema) {
}
} // namespace db
