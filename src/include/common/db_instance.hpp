#pragma once

#include "query/binder/statement/create_statement.hpp"
#include "storage/buffer/buffer_pool.hpp"
#include "meta/catalog.hpp"
#include "common/config.hpp"
#include "concurrency/transaction.hpp"
#include "concurrency/transaction_manager.hpp"
#include "query/execution_engine.hpp"

#include <memory>
#include <string>
namespace db {

class DB {
public:
	explicit DB([[maybe_unused]] const std::string &db_file_name)
	    : catalog_(std::make_unique<Catalog>()),
	      disk_manager_(std::make_shared<DiskManager>(*catalog_)),
	      bpm_(std::make_unique<BufferPool>(DEFAULT_POOL_SIZE, *disk_manager_)),
	      execution_engine_(std::make_unique<ExecutionEngine>()),
	      txn_manager_(std::make_unique<TransactionManager>()) {
	          // DeletePathIfExists(db::FilePathManager::GetInstance().GetDatabaseRootPath());
	      };
	~DB() = default;
	;

	void HandleCreateStatement(Transaction &txn, const std::unique_ptr<CreateStatement> &stmt);
	void ExecuteQuery([[maybe_unused]] Transaction &txn, const std::string &query);

private:
	void SetUpInternalSystemCatalogTable();

	std::unique_ptr<Catalog> catalog_;
	std::shared_ptr<DiskManager> disk_manager_;
	std::unique_ptr<BufferPool> bpm_;
	std::unique_ptr<ExecutionEngine> execution_engine_;

	/** Lock for Catalog */
	std::shared_mutex catalog_lock_;

public:
	std::unique_ptr<TransactionManager> txn_manager_;
};

}; // namespace db
