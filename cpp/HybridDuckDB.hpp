#pragma once

#include "HybridDuckDBSpec.hpp"
#include "HybridDatabase.hpp"
#include "duckdb.h"
#include <memory>
#include <string>

namespace margelo::nitro::duckdb {

/**
 * Implementation of the DuckDB HybridObject
 * Factory for creating database connections
 */
class HybridDuckDB : public HybridDuckDBSpec {
public:
  HybridDuckDB() : HybridObject(TAG), HybridDuckDBSpec() {}

  ~HybridDuckDB() override = default;

  // Properties
  std::string getVersion() override;
  std::string getPlatform() override;

  // Methods
  std::shared_ptr<HybridDatabaseSpec> open(
      const std::string& path,
      const std::optional<DatabaseOptions>& options) override;

  std::shared_ptr<HybridDatabaseSpec> openInMemory(
      const std::optional<DatabaseOptions>& options) override;

private:
  duckdb_config createConfig(const std::optional<DatabaseOptions>& options);
};

} // namespace margelo::nitro::duckdb
