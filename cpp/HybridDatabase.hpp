#pragma once

#include "HybridDatabaseSpec.hpp"
#include "HybridPreparedStatement.hpp"
#include "duckdb.h"
#include <NitroModules/Promise.hpp>
#include <memory>
#include <mutex>
#include <string>

namespace margelo::nitro::duckdb {

/**
 * Implementation of a DuckDB database connection
 */
class HybridDatabase : public HybridDatabaseSpec {
public:
  HybridDatabase(duckdb_database db, const std::string& path, bool readOnly);
  ~HybridDatabase() override;

  // Properties
  std::string getPath() override;
  bool getIsOpen() override;
  bool getIsReadOnly() override;

  // Methods
  std::shared_ptr<Promise<QueryResult>> execute(const std::string& sql) override;
  QueryResult executeSync(const std::string& sql) override;

  std::shared_ptr<HybridPreparedStatementSpec> prepare(
      const std::string& sql) override;

  std::shared_ptr<Promise<void>> beginTransaction() override;
  std::shared_ptr<Promise<void>> commit() override;
  std::shared_ptr<Promise<void>> rollback() override;

  std::shared_ptr<Promise<void>> loadExtension(const std::string& extensionName) override;
  std::shared_ptr<Promise<void>> installExtension(const std::string& extensionName) override;

  void close() override;

private:
  duckdb_database _db;
  duckdb_connection _conn;
  std::string _path;
  bool _isOpen;
  bool _isReadOnly;
  std::mutex _mutex;

  QueryResult executeInternal(const std::string& sql);
  void executeSimple(const std::string& sql);
  QueryResult resultToQueryResult(duckdb_result& result);
};

} // namespace margelo::nitro::duckdb
