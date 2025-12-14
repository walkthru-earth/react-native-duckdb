#pragma once

#include "HybridPreparedStatementSpec.hpp"
#include "duckdb.h"
#include <NitroModules/Promise.hpp>
#include <mutex>

namespace margelo::nitro::duckdb {

/**
 * Implementation of a DuckDB prepared statement
 */
class HybridPreparedStatement : public HybridPreparedStatementSpec {
public:
  HybridPreparedStatement(duckdb_prepared_statement stmt,
                          duckdb_connection conn);
  ~HybridPreparedStatement() override;

  // Bind methods
  void bindString(double index, const std::string& value) override;
  void bindNumber(double index, double value) override;
  void bindBoolean(double index, bool value) override;
  void bindBigInt(double index, int64_t value) override;
  void bindBlob(double index,
                const std::shared_ptr<ArrayBuffer>& value) override;
  void bindNull(double index) override;

  // Execute methods
  std::shared_ptr<Promise<QueryResult>> execute() override;
  QueryResult executeSync() override;

  // Utility methods
  void reset() override;
  void close() override;

private:
  duckdb_prepared_statement _stmt;
  duckdb_connection _conn;
  bool _isClosed;
  std::mutex _mutex;

  QueryResult executeInternal();
  QueryResult resultToQueryResult(duckdb_result& result);
};

} // namespace margelo::nitro::duckdb
