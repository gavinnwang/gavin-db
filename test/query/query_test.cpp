#include "common/db_instance.hpp"
#include "concurrency/transaction.hpp"

#include "gtest/gtest.h"
TEST(QueryTest, SimpleCreateTest) {
	db::DB db = db::DB {"test_db"};
	db::Transaction txn {};
	std::string query = R"(
INSERT INTO Users 
VALUES
    (1, 25, 50000, 'Alice');
)";
	try {
		db.ExecuteQuery(txn, query);
	} catch (const std::exception &e) {
	}

	// query = "CREATE TABLE Users ( id INT PRIMARY KEY, name VARCHAR(255) NOT NULL);";
	// try {
	// 	db.ExecuteQuery(txn, query);
	// } catch (const std::exception &e) {
	// }
	//
	// query = "CREATE TABLE Customers ( id INT PRIMARY KEY, name VARCHAR(255) NOT NULL);";
	// try {
	// 	db.ExecuteQuery(txn, query);
	// } catch (const std::exception &e) {
	// }
} // namespace db
