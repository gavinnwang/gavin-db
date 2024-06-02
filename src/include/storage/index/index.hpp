#pragma once

#include "catalog/schema.hpp"

#include <unordered_set>
namespace db {
enum class IndexConstraintType : uint8_t {
	NONE = 0,    // index is an index don't built to any constraint
	UNIQUE = 1,  // index is an index built to enforce a UNIQUE constraint
	PRIMARY = 2, // index is an index built to enforce a PRIMARY KEY constraint
	FOREIGN = 3  // index is an index built to enforce a FOREIGN KEY constraint
};
class Index {
public:
	Index(const std::string &name, const std::string &index_type, IndexConstraintType index_constraint_type,
	      const std::vector<column_t> &column_ids);

	virtual ~Index() = default;

private:
	std::string name_;
	std::string table_name_;
	std::shared_ptr<Schema> key_schema_;
	bool is_primary_key_;
	const std::unordered_set<uint32_t> key_attrs_;
};

} // namespace db
