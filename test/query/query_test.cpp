#include "SQLParser.h"
#include "util/sqlhelper.h"

#include "gtest/gtest.h"
namespace db {
TEST(QueryTest, SimpleQueryTest) {
	std::string query = "select * from car;";
	hsql::SQLParserResult result;
	hsql::SQLParser::parse(query, &result);
	if (result.isValid()) {
		printf("Parsed successfully!\n");
		printf("Number of statements: %lu\n", result.size());

		for (auto i = 0u; i < result.size(); ++i) {
			// Print a statement summary.
			hsql::printStatementInfo(result.getStatement(i));
		}
	} else {
		printf("failed!\n");
	}
}
} // namespace db
