#include "SQLParser.h"
#include "common/db_instance.hpp"
#include "concurrency/transaction.hpp"
#include "util/sqlhelper.h"

#include "gtest/gtest.h"
TEST(QueryTest, SimpleQueryTest) {
	// std::string query = "SELECT * FROM Customers WHERE CustomerID > 80;";
	std::string query = "CREATE TABLE Users ( id INT PRIMARY KEY, name VARCHAR(255) NOT NULL);";
	hsql::SQLParserResult result;
	hsql::SQLParser::parse(query, &result);

	if (result.isValid() && result.size() > 0) {
		const hsql::SQLStatement *statement = result.getStatement(0);
		hsql::printStatementInfo(statement);
	} else {
		std::cout << "what" << result.size() << result.errorMsg();
	}
}

TEST(QueryTest, SimpleCreateTest) {
	db::DB db = db::DB {"test_db"};
	db::Transaction txn {};
	std::string query = "CREATE TABLE Users ( id INT PRIMARY KEY, name VARCHAR(255) NOT NULL);";
	db.ExecuteQuery(txn, query);
} // namespace db
