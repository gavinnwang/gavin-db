#include "parser/parser.hpp"

#include "SQLParser.h"
#include "catalog/catalog_manager.hpp"
#include "common/exception.hpp"
#include "common/logger.hpp"
#include "fmt/format.h"
#include "util/sqlhelper.h"

namespace db {

const std::vector<std::unique_ptr<const hsql::SQLStatement>> Parser::Parse(const std::string &query) const {

	hsql::SQLParserResult parse_result;
	hsql::SQLParser::parse(query, &parse_result);
	if (!parse_result.isValid()) {
		LOG_INFO("Query failed to parse!");
		throw Exception(fmt::format("Query failed to parse: {}", parse_result.errorMsg()));
	}

	std::vector<std::unique_ptr<const hsql::SQLStatement>> result;

	for (const auto stmt : parse_result.getStatements()) {
		hsql::printStatementInfo(stmt);
		result.emplace_back(stmt); // Transfer ownership
	}
	return result;
}
} // namespace db
