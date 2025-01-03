#pragma once

#include "binder/statement/create_statement.hpp"
#include "buffer/buffer_pool_manager.hpp"
#include "catalog/catalog_manager.hpp"
#include "common/config.hpp"
#include "concurrency/transaction.hpp"
#include "execution/execution_engine.hpp"

#include <memory>
#include <string>
namespace db {

class DB {
public:
	explicit DB([[maybe_unused]] const std::string &db_file_name)
	    : catalog_manager_(std::make_unique<CatalogManager>()),
	      disk_manager_(std::make_shared<DiskManager>(catalog_manager_)),
	      bpm_(std::make_unique<BufferPoolManager>(DEFAULT_POOL_SIZE, disk_manager_)),
	      execution_engine_(std::make_unique<ExecutionEngine>()) {
	          // DeletePathIfExists(db::FilePathManager::GetInstance().GetDatabaseRootPath());
	      };
	~DB() = default;;

	void HandleCreateStatement(Transaction &txn, const std::unique_ptr<CreateStatement> &stmt);
	void ExecuteQuery([[maybe_unused]] Transaction &txn, const std::string &query);

private:
	void SetUpInternalSystemCatalogTable();

	std::unique_ptr<CatalogManager> catalog_manager_;
	std::shared_ptr<DiskManager> disk_manager_;
	std::unique_ptr<BufferPoolManager> bpm_;
	std::unique_ptr<ExecutionEngine> execution_engine_;

	/** Lock for CatalogManager */
	std::shared_mutex catalog_manager_lock_;
};

}; // namespace db
