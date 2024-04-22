#include "common/db_instance.hpp"
#include "storage/disk.hpp"
#include "storage/kv_page.hpp"
#include "storage/page.hpp"
#include <iostream>
#include <memory>
#include <sstream>

namespace db {

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

DBInstance::DBInstance(const std::string &db_file_name) {
  disk_manager_ =  std::make_unique<DiskManager>(db_file_name);
  // auto new_page = Page();
  // disk_manager_->ReadPage(0, new_page.GetData());
  // auto kv_page = new_page.AsMut<KVPage>();
  // if(!kv_page->Put("Hi", "wow")) {
  //   std::cout << "put failed";
  // }
  // std::string val;
  // if(kv_page->Get("Hi", &val)) {
  //    std::cout << val << std::endl;
  // }
  // // kv_page->PrintContent();
  // disk_manager_->WritePage(0, new_page.GetData());
  // disk_manager_->ShutDown();
};

auto DBInstance::ExecuteQuery(const std::string &query, std::string* output) -> bool{
  auto parsed = split(query, ' ');
  if (!parsed.empty()) {
    std::string& lastStr = parsed.back();
    if (!lastStr.empty()) {
      lastStr.pop_back();
    }
   if (parsed[0] == "get") {
      if (parsed.size() != 2) {
        return false;
      }
      auto new_page = Page();
      disk_manager_->ReadPage(0, new_page.GetData());
      auto kv_page = new_page.AsMut<KVPage>();
      return kv_page->Get(parsed[1], output);
    }
  else if (parsed[0] == "put") {
     if (parsed.size() != 3) {
        return false;
      }
      auto new_page = Page();
      disk_manager_->ReadPage(0, new_page.GetData());
      auto kv_page = new_page.AsMut<KVPage>();
      auto res = kv_page->Put(parsed[1], parsed[2]);
      disk_manager_->WritePage(0, new_page.GetData());
      // kv_page->PrintContent();
      return res;
    }
  }
  return false;
}
}
