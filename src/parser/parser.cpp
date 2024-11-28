#include "parser/parser.hpp"

#include "SQLParser.h"
#include "common/exception.hpp"
#include "common/logger.hpp"
#include "fmt/format.h"

namespace db {

const std::vector<std::unique_ptr<const hsql::SQLStatement>> Parser::Parse(std::string query) {

	hsql::SQLParserResult parse_result;
	hsql::SQLParser::parse(query, &parse_result);
	if (!parse_result.isValid()) {
		LOG_INFO("Query failed to parse!");
		throw Exception(fmt::format("Query failed to parse: {}", parse_result.errorMsg()));
	}

	std::vector<std::unique_ptr<const hsql::SQLStatement>> result;

	for (auto stmt : parse_result.getStatements()) {
		result.emplace_back(stmt); // Transfer ownership
	}
	return result;
}
} // namespace db
