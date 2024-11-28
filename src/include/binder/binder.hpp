#include "SQLParser.h"
#include "catalog/catalog_manager.hpp"

namespace db {
class Binder {
public:
	explicit Binder(const CatalogManager &CatalogManager) {
	}

private:
	[[maybe_unused]] hsql::SQLParser parser_;
};
} // namespace db
