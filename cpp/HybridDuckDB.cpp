#include "HybridDuckDB.hpp"
#include "HybridDatabase.hpp"
#include <stdexcept>

namespace margelo::nitro::duckdb {

std::string HybridDuckDB::getVersion() {
  return duckdb_library_version();
}

std::string HybridDuckDB::getPlatform() {
#if defined(__APPLE__)
  return "ios";
#elif defined(__ANDROID__)
  return "android";
#else
  return "unknown";
#endif
}

duckdb_config HybridDuckDB::createConfig(
    const std::optional<DatabaseOptions>& options) {
  duckdb_config config = nullptr;

  if (options.has_value()) {
    if (duckdb_create_config(&config) == DuckDBError) {
      throw std::runtime_error("Failed to create DuckDB config");
    }

    const auto& opts = options.value();

    if (opts.readOnly.has_value() && opts.readOnly.value()) {
      duckdb_set_config(config, "access_mode", "read_only");
    }

    if (opts.threads.has_value()) {
      auto threads = std::to_string(opts.threads.value());
      duckdb_set_config(config, "threads", threads.c_str());
    }

    if (opts.maxMemory.has_value()) {
      duckdb_set_config(config, "max_memory", opts.maxMemory.value().c_str());
    }

    if (opts.accessMode.has_value()) {
      duckdb_set_config(config, "access_mode", opts.accessMode.value().c_str());
    }
  }

  return config;
}

std::shared_ptr<HybridDatabaseSpec> HybridDuckDB::open(
    const std::string& path,
    const std::optional<DatabaseOptions>& options) {

  duckdb_database db = nullptr;
  duckdb_config config = createConfig(options);
  char* error = nullptr;

  duckdb_state state;
  if (config != nullptr) {
    state = duckdb_open_ext(path.c_str(), &db, config, &error);
    duckdb_destroy_config(&config);
  } else {
    state = duckdb_open(path.c_str(), &db);
  }

  if (state == DuckDBError) {
    std::string errorMsg = error ? error : "Unknown error opening database";
    if (error) {
      duckdb_free(error);
    }
    throw std::runtime_error(errorMsg);
  }

  bool isReadOnly = options.has_value() &&
                    options.value().readOnly.has_value() &&
                    options.value().readOnly.value();

  return std::make_shared<HybridDatabase>(db, path, isReadOnly);
}

std::shared_ptr<HybridDatabaseSpec> HybridDuckDB::openInMemory(
    const std::optional<DatabaseOptions>& options) {
  return open(":memory:", options);
}

} // namespace margelo::nitro::duckdb
