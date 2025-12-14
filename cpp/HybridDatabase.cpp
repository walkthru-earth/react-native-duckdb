#include "HybridDatabase.hpp"
#include "HybridPreparedStatement.hpp"
#include <NitroModules/Promise.hpp>
#include <stdexcept>
#include <sstream>
#include <thread>

namespace margelo::nitro::duckdb {

HybridDatabase::HybridDatabase(duckdb_database db, const std::string& path,
                               bool readOnly)
    : HybridObject(TAG), HybridDatabaseSpec(), _db(db), _conn(nullptr),
      _path(path), _isOpen(true), _isReadOnly(readOnly) {

  if (duckdb_connect(db, &_conn) == DuckDBError) {
    duckdb_close(&_db);
    throw std::runtime_error("Failed to create database connection");
  }
}

HybridDatabase::~HybridDatabase() {
  close();
}

std::string HybridDatabase::getPath() {
  return _path;
}

bool HybridDatabase::getIsOpen() {
  return _isOpen;
}

bool HybridDatabase::getIsReadOnly() {
  return _isReadOnly;
}

QueryResult HybridDatabase::resultToQueryResult(duckdb_result& result) {
  QueryResult qr;

  idx_t columnCount = duckdb_column_count(&result);
  idx_t rowCount = duckdb_row_count(&result);

  // Get column names
  for (idx_t i = 0; i < columnCount; i++) {
    qr.columns.push_back(duckdb_column_name(&result, i));
  }

  // Build JSON array of rows
  std::ostringstream json;
  json << "[";

  for (idx_t row = 0; row < rowCount; row++) {
    if (row > 0) json << ",";
    json << "[";

    for (idx_t col = 0; col < columnCount; col++) {
      if (col > 0) json << ",";

      if (duckdb_value_is_null(&result, col, row)) {
        json << "null";
        continue;
      }

      duckdb_type type = duckdb_column_type(&result, col);

      switch (type) {
        case DUCKDB_TYPE_BOOLEAN: {
          bool val = duckdb_value_boolean(&result, col, row);
          json << (val ? "true" : "false");
          break;
        }
        case DUCKDB_TYPE_TINYINT:
        case DUCKDB_TYPE_SMALLINT:
        case DUCKDB_TYPE_INTEGER:
        case DUCKDB_TYPE_BIGINT: {
          int64_t val = duckdb_value_int64(&result, col, row);
          json << val;
          break;
        }
        case DUCKDB_TYPE_UTINYINT:
        case DUCKDB_TYPE_USMALLINT:
        case DUCKDB_TYPE_UINTEGER:
        case DUCKDB_TYPE_UBIGINT: {
          uint64_t val = duckdb_value_uint64(&result, col, row);
          json << val;
          break;
        }
        case DUCKDB_TYPE_FLOAT:
        case DUCKDB_TYPE_DOUBLE: {
          double val = duckdb_value_double(&result, col, row);
          json << val;
          break;
        }
        case DUCKDB_TYPE_VARCHAR: {
          char* val = duckdb_value_varchar(&result, col, row);
          if (val) {
            // Escape JSON string
            json << "\"";
            for (const char* p = val; *p; p++) {
              switch (*p) {
                case '"': json << "\\\""; break;
                case '\\': json << "\\\\"; break;
                case '\n': json << "\\n"; break;
                case '\r': json << "\\r"; break;
                case '\t': json << "\\t"; break;
                default: json << *p; break;
              }
            }
            json << "\"";
            duckdb_free(val);
          } else {
            json << "null";
          }
          break;
        }
        default: {
          // For unsupported types, convert to string
          char* val = duckdb_value_varchar(&result, col, row);
          if (val) {
            json << "\"" << val << "\"";
            duckdb_free(val);
          } else {
            json << "null";
          }
          break;
        }
      }
    }
    json << "]";
  }
  json << "]";

  qr.rowsJson = json.str();
  qr.rowCount = static_cast<double>(rowCount);
  qr.rowsAffected = static_cast<double>(duckdb_rows_changed(&result));

  return qr;
}

QueryResult HybridDatabase::executeInternal(const std::string& sql) {
  std::lock_guard<std::mutex> lock(_mutex);

  if (!_isOpen) {
    throw std::runtime_error("Database is closed");
  }

  duckdb_result result;
  if (duckdb_query(_conn, sql.c_str(), &result) == DuckDBError) {
    std::string error = duckdb_result_error(&result);
    duckdb_destroy_result(&result);
    throw std::runtime_error(error);
  }

  QueryResult qr = resultToQueryResult(result);
  duckdb_destroy_result(&result);
  return qr;
}

void HybridDatabase::executeSimple(const std::string& sql) {
  std::lock_guard<std::mutex> lock(_mutex);

  if (!_isOpen) {
    throw std::runtime_error("Database is closed");
  }

  duckdb_result result;
  if (duckdb_query(_conn, sql.c_str(), &result) == DuckDBError) {
    std::string error = duckdb_result_error(&result);
    duckdb_destroy_result(&result);
    throw std::runtime_error(error);
  }
  duckdb_destroy_result(&result);
}

std::shared_ptr<Promise<QueryResult>> HybridDatabase::execute(const std::string& sql) {
  return Promise<QueryResult>::async([this, sql]() {
    return executeInternal(sql);
  });
}

QueryResult HybridDatabase::executeSync(const std::string& sql) {
  return executeInternal(sql);
}

std::shared_ptr<HybridPreparedStatementSpec> HybridDatabase::prepare(
    const std::string& sql) {
  std::lock_guard<std::mutex> lock(_mutex);

  if (!_isOpen) {
    throw std::runtime_error("Database is closed");
  }

  duckdb_prepared_statement stmt = nullptr;
  if (duckdb_prepare(_conn, sql.c_str(), &stmt) == DuckDBError) {
    std::string error = duckdb_prepare_error(stmt);
    duckdb_destroy_prepare(&stmt);
    throw std::runtime_error(error);
  }

  return std::make_shared<HybridPreparedStatement>(stmt, _conn);
}

std::shared_ptr<Promise<void>> HybridDatabase::beginTransaction() {
  return Promise<void>::async([this]() {
    executeSimple("BEGIN TRANSACTION");
  });
}

std::shared_ptr<Promise<void>> HybridDatabase::commit() {
  return Promise<void>::async([this]() {
    executeSimple("COMMIT");
  });
}

std::shared_ptr<Promise<void>> HybridDatabase::rollback() {
  return Promise<void>::async([this]() {
    executeSimple("ROLLBACK");
  });
}

std::shared_ptr<Promise<void>> HybridDatabase::loadExtension(
    const std::string& extensionName) {
  return Promise<void>::async([this, extensionName]() {
    executeSimple("LOAD '" + extensionName + "'");
  });
}

std::shared_ptr<Promise<void>> HybridDatabase::installExtension(
    const std::string& extensionName) {
  return Promise<void>::async([this, extensionName]() {
    executeSimple("INSTALL '" + extensionName + "'");
  });
}

void HybridDatabase::close() {
  std::lock_guard<std::mutex> lock(_mutex);

  if (_isOpen) {
    if (_conn != nullptr) {
      duckdb_disconnect(&_conn);
      _conn = nullptr;
    }
    if (_db != nullptr) {
      duckdb_close(&_db);
      _db = nullptr;
    }
    _isOpen = false;
  }
}

} // namespace margelo::nitro::duckdb
