// #include <common/db_instance.hpp>
// #include <iostream>
//
// auto EndsWith(const std::string &str, const std::string &suffix) -> bool {
// 	if (suffix.size() > str.size()) {
// 		return false;
// 	}
// 	return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
// }
//
// int main() {
// 	db::DBInstance db("test.db");
// 	std::cout << "Welcome to the gavin db!" << std::endl << std::endl;
// 	while (true) {
// 		std::string query;
// 		while (true) {
// 			std::string query_line;
// 			std::cout << "> ";
// 			std::getline(std::cin, query_line);
// 			if (!std::cin) {
// 				return 0;
// 			}
// 			query += query_line;
// 			if (EndsWith(query, ";")) {
// 				break;
// 			}
// 			query += "\n";
// 		}
// 		std::string output;
// 		if (db.ExecuteQuery(query, &output)) {
// 			std::cout << output << std::endl;
// 		} else {
// 			std::cerr << output << std::endl;
// 		}
// 	}
// }
