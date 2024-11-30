#include "common/db_instance.hpp"

#include "SQLParser.h"
#include "binder/binder.hpp"
#include "binder/statement/create_statement.hpp"
#include "catalog/schema.hpp"
#include "common/exception.hpp"
#include "common/macros.hpp"
#include "util/sqlhelper.h"

#include <algorithm>
namespace db {

void DB::ExecuteQuery([[maybe_unused]] Transaction &txn, const std::string &query) {
	ASSERT(catalog_manager_ && bpm_, "catalog manager and buffer pool manager must be initialized");
	hsql::SQLParserResult raw_parse_result;
	hsql::SQLParser::parse(query, &raw_parse_result);
	if (!raw_parse_result.isValid()) {
		LOG_INFO("Query failed to parse!");
		throw Exception(fmt::format("Query failed to parse: {}", raw_parse_result.errorMsg()));
	}
	const auto binder = Binder {catalog_manager_};
	for (const auto &parsed_stmt : raw_parse_result.getStatements()) {
		std::cout << parsed_stmt << std::endl;
		hsql::printStatementInfo(parsed_stmt);
		auto binded_stmt = binder.Bind(parsed_stmt);
		switch (binded_stmt->type_) {
		// case StatementType::INVALID_STATEMENT:
		// case StatementType::SELECT_STATEMENT:
		// case StatementType::INSERT_STATEMENT:
		case StatementType::CREATE_STATEMENT: {
			auto *raw_create_stmt = static_cast<CreateStatement *>(binded_stmt.release());
			std::unique_ptr<CreateStatement> create_stmt(raw_create_stmt);
			HandleCreateStatement(txn, create_stmt);
			continue;
		}
		default: {
			throw NotImplementedException("Not implemented statement");
		}
		}
	}
	std::cout << "EXECUTED SUCCCESS" << std::endl;
}

void DB::HandleCreateStatement([[maybe_unused]] Transaction &txn, const std::unique_ptr<CreateStatement> &stmt) {
	std::unique_lock<std::shared_mutex> l(catalog_manager_lock_);
	const auto schema = Schema {stmt->columns_};
	const auto &table_meta = catalog_manager_->CreateTable(stmt->table_name_, schema);
	if (!table_meta.has_value()) {
		throw RuntimeException("Failed to create table: table already exists");
	}
	const auto &table_name = stmt->table_name_;
	if (!stmt->primary_key_.empty()) {
		ASSERT(stmt->primary_key_.size() == 1, "Only support one primary key");
		const auto &primary_key = stmt->primary_key_.at(0);
		const auto key_col_it =
		    std::ranges::find_if(schema.GetColumns(), [&](const Column col) { return col.GetName() == primary_key; });
		ASSERT(key_col_it != schema.GetColumns().end(), "Broken invariant pk col not found");
		const auto index_oid = catalog_manager_->CreateIndex(table_name + "_pk", table_name, *key_col_it, true,
		                                                     IndexType::BPlusTreeIndex, bpm_);
		if (!index_oid.has_value()) {
			throw RuntimeException("Failed to create primary key index");
		}
	}
	l.unlock();
	std::cout << "Create statement success" << std::endl;
}

void DB::SetUpInternalSystemCatalogTable() {
	// check if it already exist
	auto sys_catalog_path = FilePathManager::GetInstance().GetSystemCatalogPath();

	if (fs::exists(sys_catalog_path)) {
		// if already exist
	} else {
		// need to set up
		CreateFileIfNotExists(sys_catalog_path);
	}
}
} // namespace db
