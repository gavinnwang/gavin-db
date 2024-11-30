#pragma once

#include "sql/SQLStatement.h"
namespace db {

class Parser {

public:
	explicit Parser() {
	}

	const std::vector<std::unique_ptr<const hsql::SQLStatement>> Parse(const std::string &query) const;
};

} // namespace db
