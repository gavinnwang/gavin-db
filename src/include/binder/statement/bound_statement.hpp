#pragma once
#include "binder/statement/statement_type.hpp"

#include <string>

namespace db {

class BoundStatement {
public:
	explicit BoundStatement(StatementType type) : type_(type) {};
	virtual ~BoundStatement() = default;

	/** The statement type. */
	const StatementType type_;

public:
	virtual auto ToString() const -> std::string = 0;
};
} // namespace db
