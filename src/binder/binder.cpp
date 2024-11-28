#include "binder/binder.hpp"

#include "SQLParser.h"
#include "SQLParserResult.h"
#include "fmt/core.h"

namespace db {
void Binder::Parse(const std::string &query) {
	hsql::SQLParserResult result;
	hsql::SQLParser::parse(query, &result);
	if (!result.isValid()) {
		LOG_INFO("Query failed to parse!");
		throw Exception(fmt::format("Query failed to parse: {}", result.errorMsg()));
		return;
	}

	if (result.size() == 0) {
		LOG_INFO("parser received empty statement");
		return;
	}
	statement_nodes_ = result.getStatements();
}
} // namespace db
