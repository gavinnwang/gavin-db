#include "storage/catalog_page.hpp"

#include <cstring>
namespace db {
void CatalogPage::Init(BufferPoolManager *bpm) {
	uint32_t existing_magic;
	memcpy(&existing_magic, page_start_, sizeof(MAGIC_NUMBER));
	if (existing_magic != MAGIC_NUMBER) {
		// If MAGIC_NUMBER is not found, initialize the page
		memcpy(page_start_, &MAGIC_NUMBER, sizeof(MAGIC_NUMBER));

		next_page_id_ = INVALID_PAGE_ID;
		// tables_table_page_id_ = INVALID_PAGE_ID;
		// indexes_table_page_id_ = INVALID_PAGE_ID;
		// auto table_page_guard = bpm->NewPageGuarded(&tables_table_page_id_);
		// auto write_guard = table_page_guard.UpgradeWrite();
		// auto table_page auto index_page_guard =
		//     bpm->NewPageGuarded(&indexes_table_page_id_);

	} else {
		// The page has already been initialized
		// ASSERT
	}
}
} // namespace db
