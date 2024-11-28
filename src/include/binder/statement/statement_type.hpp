#pragma once
#include <cstdint>
namespace db {

enum class StatementType : uint8_t {
	INVALID_STATEMENT, // invalid statement type
	SELECT_STATEMENT,  // select statement type
	INSERT_STATEMENT,  // insert statement type
	CREATE_STATEMENT,  // create statement type
};

}
