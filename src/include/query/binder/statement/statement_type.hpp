#pragma once
#include <cstdint>
namespace db {

enum class StatementType : uint8_t {
	SELECT_STATEMENT, // select statement type
	INSERT_STATEMENT, // insert statement type
	CREATE_STATEMENT, // create statement type
};

}
