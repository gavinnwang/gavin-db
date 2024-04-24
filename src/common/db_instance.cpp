#include "common/db_instance.hpp"
#include "buffer/buffer_pool_manager.hpp"
#include "storage/disk.hpp"
#include "storage/kv_page.hpp"
#include "storage/page.hpp"
#include <memory>
#include <sstream>

namespace db {

std::vector<std::string> split(const std::string &str, char delimiter) {
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(str);
  while (std::getline(tokenStream, token, delimiter)) {
    tokens.push_back(token);
  }
  return tokens;
}

DBInstance::DBInstance(const std::string &db_file_name) {
  disk_manager_ = std::make_unique<DiskManager>(db_file_name);
  buffer_pool_manager_ = std::make_unique<BufferPoolManager>(
      DEFAULT_POOL_SIZE, disk_manager_.get());
};

DBInstance::~DBInstance() { disk_manager_->ShutDown(); }

auto DBInstance::ExecuteQuery(const std::string &query,
                              std::string *output) -> bool {
  auto parsed = split(query, ' ');
  if (!parsed.empty()) {
    std::string &lastStr = parsed.back();
    if (!lastStr.empty()) {
      lastStr.pop_back();
    }
    if (parsed[0] == "print") {
      auto new_page = Page();
      disk_manager_->ReadPage(0, new_page.GetData());
      auto kv_page = new_page.As<KVPage>();
      kv_page->PrintContent();
      return true;
    }
    if (parsed[0] == "compact") {
      auto new_page = Page();
      disk_manager_->ReadPage(0, new_page.GetData());
      auto kv_page = new_page.AsMut<KVPage>();
      kv_page->Compact();
      disk_manager_->WritePage(0, new_page.GetData());
      return true;
    }
    if (parsed[0] == "get") {
      if (parsed.size() != 2) {
        *output = "parse error";
        return false;
      }
      auto new_page = Page();
      disk_manager_->ReadPage(0, new_page.GetData());
      auto kv_page = new_page.As<KVPage>();
      return kv_page->Get(parsed[1], output);
    }
    if (parsed[0] == "put") {
      if (parsed.size() != 3) {
        *output = "parse error";
        return false;
      }
      auto new_page = Page();
      disk_manager_->ReadPage(0, new_page.GetData());
      auto kv_page = new_page.AsMut<KVPage>();
      auto res = kv_page->Put(parsed[1], parsed[2]);
      disk_manager_->WritePage(0, new_page.GetData());
      if (res) {
        *output = "put success";
      }
      return res;
    }
    if (parsed[0] == "delete") {
      if (parsed.size() != 2) {
        *output = "parse error";
        return false;
      }
      auto new_page = Page();
      disk_manager_->ReadPage(0, new_page.GetData());
      auto kv_page = new_page.AsMut<KVPage>();
      auto res = kv_page->Delete(parsed[1]);
      disk_manager_->WritePage(0, new_page.GetData());
      if (res) {
        *output = "delete success";
      }
      return res;
    }
  }
  *output = "parse error";
  return false;
}
} // namespace db
