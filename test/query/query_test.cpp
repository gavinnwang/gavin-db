#include "SQLParser.h"
#include "util/sqlhelper.h"

#include "gtest/gtest.h"
namespace db {
TEST(QueryTest, SimpleQueryTest) {
	std::string query = "select * from car;";
	hsql::SQLParserResult result;
	hsql::SQLParser::parse(query, &result);

	if (result.isValid() && result.size() > 0) {
		const hsql::SQLStatement *statement = result.getStatement(0);

		if (statement->isType(hsql::kStmtSelect)) {
			const auto *select = static_cast<const hsql::SelectStatement *>(statement);
			std::cout << select->fromTable->getName() << std::endl;
		}
	}
}
} // namespace db
