#include "common/db_instance.hpp"
#include "storage/disk.hpp"
#include "storage/kv_page.hpp"
#include "storage/page.hpp"
#include <memory>

namespace db {

DBInstance::DBInstance(const std::string &db_file_name) {
  disk_manager_ =  std::make_unique<DiskManager>(db_file_name);
  auto new_page = Page();

  disk_manager_->ReadPage(0, new_page.GetData());

  auto kv_page = new_page.AsMut<KVPage>();
  kv_page->Put("Hi", "sup");
  kv_page->PrintContent();
  
  disk_manager_->WritePage(0, new_page.GetData());
  disk_manager_->ShutDown();
};
}
