#include "common/db_instance.hpp"
#include "concurrency/transaction.hpp"

#include "gtest/gtest.h"
namespace db {
TEST(QueryTest, SimpleCreateTest) {
	DB db = DB {"test_db"};
	Transaction txn {1, IsolationLevel::READ_UNCOMMITTED};
	std::string query = R"(
	CREATE TABLE Users (id INT PRIMARY KEY, age INT, networth INT, name VARCHAR(255));
		)";
	try {
		db.ExecuteQuery(txn, query);
	} catch (const std::exception &e) {
	}

	query = R"(
INSERT INTO Users 
SELECT 2, 25, 50000, 'FOWEIFJWOEFIJWEOIFJWEOIJFWOEIFJWEOIJFEWOWOEFIJ';
	)";
	try {
		db.ExecuteQuery(txn, query);
	} catch (const std::exception &e) {
	}

	query = R"(
SELECT * FROM Users;
	)";
	try {
		db.ExecuteQuery(txn, query);
	} catch (const std::exception &e) {
	}

	// query = "CREATE TABLE Customers ( id INT PRIMARY KEY, name VARCHAR(255) NOT NULL);";
	// try {
	// 	db.ExecuteQuery(txn, query);
	// } catch (const std::exception &e) {
	// }
}
} // namespace db
