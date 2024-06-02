// #include "common/db_instance.hpp"
//
// #include "buffer/buffer_pool_manager.hpp"
// #include "storage/disk_manager.hpp"
// #include "storage/kv_page.hpp"
// #include "storage/page.hpp"
//
// #include <memory>
// #include <sstream>
//
// namespace db {
//
// std::vector<std::string> split(const std::string &str, char delimiter) {
// 	std::vector<std::string> tokens;
// 	std::string token;
// 	std::istringstream tokenStream(str);
// 	while (std::getline(tokenStream, token, delimiter)) {
// 		tokens.push_back(token);
// 	}
// 	return tokens;
// }
//
// DBInstance::DBInstance(const std::string &db_file_name) {
// 	auto disk_manager = std::make_unique<DiskManager>(db_file_name);
// 	buffer_pool_manager_ = std::make_shared<BufferPoolManager>(DEFAULT_POOL_SIZE, std::move(disk_manager), 0);
// };
//
// DBInstance::~DBInstance() {
// 	buffer_pool_manager_->FlushAllPages();
// 	// disk_manager_->ShutDown();
// }
//
// auto DBInstance::ExecuteQuery(const std::string &query, std::string *output) -> bool {
// 	auto parsed = split(query, ' ');
// 	if (!parsed.empty()) {
// 		std::string &lastStr = parsed.back();
// 		if (!lastStr.empty()) {
// 			lastStr.pop_back();
// 		}
// 		if (parsed[0] == "flush") {
// 			buffer_pool_manager_->FlushAllPages();
// 			return true;
// 		}
// 		if (parsed[0] == "print") {
// 			auto page_guard = buffer_pool_manager_->FetchPageRead(0);
// 			auto kv_page = page_guard.As<KVPage>();
// 			kv_page->PrintContent();
// 			return true;
// 		}
// 		if (parsed[0] == "compact") {
// 			auto page_guard = buffer_pool_manager_->FetchPageWrite(0);
// 			auto kv_page = page_guard.AsMut<KVPage>();
// 			kv_page->Compact();
// 			return true;
// 		}
// 		if (parsed[0] == "get") {
// 			if (parsed.size() != 2) {
// 				*output = "parse error";
// 				return false;
// 			}
// 			auto page_guard = buffer_pool_manager_->FetchPageRead(0);
// 			auto kv_page = page_guard.As<KVPage>();
// 			return kv_page->Get(parsed[1], output);
// 		}
// 		if (parsed[0] == "put") {
// 			if (parsed.size() != 3) {
// 				*output = "parse error";
// 				return false;
// 			}
// 			auto page_guard = buffer_pool_manager_->FetchPageWrite(0);
// 			auto kv_page = page_guard.AsMut<KVPage>();
// 			auto res = kv_page->Put(parsed[1], parsed[2]);
// 			if (res) {
// 				*output = "put success";
// 			}
// 			return res;
// 		}
// 		if (parsed[0] == "delete") {
// 			if (parsed.size() != 2) {
// 				*output = "parse error";
// 				return false;
// 			}
// 			auto page_guard = buffer_pool_manager_->FetchPageWrite(0);
// 			auto kv_page = page_guard.AsMut<KVPage>();
// 			auto res = kv_page->Delete(parsed[1]);
// 			if (res) {
// 				*output = "delete success";
// 			}
// 			return res;
// 		}
// 	}
// 	*output = "parse error";
// 	return false;
// }
// } // namespace db
