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
	explicit BoundBaseTableRef(std::string table, table_oid_t oid, std::optional<std::string> alias, Schema schema)
	    : BoundTableRef(TableReferenceType::BASE_TABLE), table_(std::move(table)), oid_(oid), alias_(std::move(alias)),
	      schema_(std::move(schema)) {
	}

	auto ToString() const -> std::string override {
		if (alias_.has_value()) {
			return fmt::format("BoundBaseTableRef {{ table={}, oid={}, alias={} }}", table_, oid_, *alias_);
		}
		return fmt::format("BoundBaseTableRef {{ table={}, oid={} }}", table_, oid_);
	}

	auto GetBoundTableName() const -> std::string {
		if (alias_ != std::nullopt) {
			return *alias_;
		}
		return table_;
	}

	std::string table_;
	table_oid_t oid_;
	std::optional<std::string> alias_;
	Schema schema_;
};
} // namespace db
