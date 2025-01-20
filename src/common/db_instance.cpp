#include "common/db_instance.hpp"

#include "SQLParser.h"
#include "common/exception.hpp"
#include "meta/schema.hpp"
#include "query/binder/binder.hpp"
#include "query/binder/statement/create_statement.hpp"
#include "query/execution_engine.hpp"
#include "query/executor_context.hpp"
#include "query/planner.hpp"

#include <algorithm>

namespace db {

void DB::ExecuteQuery([[maybe_unused]] Transaction &txn, const std::string &query) {
	assert(catalog_ && bpm_ && "meta manager and buffer pool manager must be initialized");
	hsql::SQLParserResult raw_parse_result;
	hsql::SQLParser::parse(query, &raw_parse_result);
	if (!raw_parse_result.isValid()) {
		LOG_INFO("Query failed to parse!");
		throw Exception(fmt::format("Query failed to parse: {}", raw_parse_result.errorMsg()));
	}
	auto binder = Binder {*catalog_};
	for (const auto &parsed_stmt : raw_parse_result.getStatements()) {
		auto bound_stmt = binder.Bind(parsed_stmt);
		LOG_DEBUG("Bound statement: {}", bound_stmt->ToString());
		switch (bound_stmt->type_) {
		case StatementType::SELECT_STATEMENT:
		case StatementType::INSERT_STATEMENT: {
			auto planner = Planner {*catalog_};
			planner.PlanQuery(*bound_stmt);
			std::vector<Tuple> result_set;
			auto context = ExecutorContext {*catalog_, *bpm_};
			execution_engine_->Execute(std::move(planner.plan_), result_set, txn, context);
			bpm_->FlushAllPages();
			catalog_->PersistToDisk();
			continue;
		}
		case StatementType::CREATE_STATEMENT: {
			auto create_stmt = *static_cast<CreateStatement *>(bound_stmt.release());
			HandleCreateStatement(txn, create_stmt);
			continue;
		}
		default: {
			throw NotImplementedException("Not implemented statement");
		}
		}
	}
	LOG_INFO("Query {} executed success.", query);
}

void DB::HandleCreateStatement([[maybe_unused]] Transaction &txn, const CreateStatement &stmt) {
	std::unique_lock<std::shared_mutex> l(catalog_lock_);
	const auto schema = Schema {stmt.columns_};
	const auto &table_oid_opt = catalog_->CreateTable(stmt.table_name_, schema);
	if (!table_oid_opt.has_value()) {
		const auto &table_meta = catalog_->GetTableByName(stmt.table_name_);
		throw RuntimeException(
		    fmt::format("Failed to create table: table already exists with schema: {}", table_meta.schema_));
	}
	const auto &table_name = stmt.table_name_;
	if (!stmt.primary_key_.empty()) {
		assert(stmt.primary_key_.size() == 1 && "Only support one primary key");
		const auto &primary_key = stmt.primary_key_.at(0);
		const auto key_col_it =
		    std::ranges::find_if(schema.GetColumns(), [&](const Column &col) { return col.GetName() == primary_key; });
		assert(key_col_it != schema.GetColumns().end() && "Broken invariant pk col not found");
		const auto index_oid =
		    catalog_->CreateIndex(table_name + "_pk", table_name, *key_col_it, true, IndexType::BPlusTreeIndex, *bpm_);
		if (!index_oid.has_value()) {
			throw RuntimeException("Failed to create primary key index");
		}
	}
	l.unlock();
	LOG_INFO("Create statement executed success");
}

void DB::SetUpInternalSystemCatalogTable() {
	// check if it already exist
	auto sys_catalogpath = FilePathManager::GetInstance().GetSystemCatalogPath();

	if (fs::exists(sys_catalogpath)) {
		// if already exist
	} else {
		// need to set up
		CreateFileIfNotExists(sys_catalogpath);
	}
}
} // namespace db
