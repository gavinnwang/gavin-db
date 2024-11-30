#pragma once
#include "binder/statement/statement_type.hpp"
#include "common/exception.hpp"
namespace db {

class BoundStatement {
public:
	explicit BoundStatement(StatementType type) : type_(type) {};
	virtual ~BoundStatement() = default;

	/** The statement type. */
	StatementType type_;

public:
	/** Render this statement as a string. */
	virtual auto ToString() const -> std::string {
		throw Exception("ToString not supported for this type of SQLStatement");
	}
};
} // namespace db
