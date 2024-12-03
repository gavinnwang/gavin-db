#pragma once

#include "binder/table_ref/bound_table_ref.hpp"
#include "catalog/schema.hpp"
#include "common/typedef.hpp"
#include "fmt/core.h"

#include <optional>

namespace db {

/**
 * A bound table ref type for single table. e.g., `SELECT x FROM y`, where `y` is `BoundBaseTableRef`.
 */
class BoundBaseTableRef : public BoundTableRef {
public:
	explicit BoundBaseTableRef(std::string table, table_oid_t oid, Schema schema)
	    : BoundTableRef(TableReferenceType::BASE_TABLE), table_(std::move(table)), oid_(oid),
	      schema_(std::move(schema)) {
	}

	[[nodiscard]] std::string ToString() const override {
		return fmt::format("BoundBaseTableRef {{ table={}, oid={} }}", table_, oid_);
	}

	[[nodiscard]] auto GetBoundTableName() const -> std::string {
		return table_;
	}

	std::string table_;
	table_oid_t oid_;
	Schema schema_;
};
} // namespace db
