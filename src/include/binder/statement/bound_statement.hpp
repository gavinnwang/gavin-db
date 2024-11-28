#pragma once
namespace db {

class BoundStatement {
public:
	explicit BoundStatement(StatementType type);
	virtual ~BoundStatement() = default;
};
} // namespace db
