#include "HybridPreparedStatement.hpp"
#include <NitroModules/Promise.hpp>
#include <stdexcept>
#include <sstream>

namespace margelo::nitro::duckdb {

HybridPreparedStatement::HybridPreparedStatement(duckdb_prepared_statement stmt,
                                                 duckdb_connection conn)
    : HybridObject(TAG), HybridPreparedStatementSpec(), _stmt(stmt),
      _conn(conn), _isClosed(false) {}

HybridPreparedStatement::~HybridPreparedStatement() {
  close();
}

void HybridPreparedStatement::bindString(double index, const std::string& value) {
  std::lock_guard<std::mutex> lock(_mutex);
  if (_isClosed) {
    throw std::runtime_error("Prepared statement is closed");
  }
  idx_t idx = static_cast<idx_t>(index);
  if (duckdb_bind_varchar(_stmt, idx, value.c_str()) == DuckDBError) {
    throw std::runtime_error("Failed to bind string parameter");
  }
}

void HybridPreparedStatement::bindNumber(double index, double value) {
  std::lock_guard<std::mutex> lock(_mutex);
  if (_isClosed) {
    throw std::runtime_error("Prepared statement is closed");
  }
  idx_t idx = static_cast<idx_t>(index);
  if (duckdb_bind_double(_stmt, idx, value) == DuckDBError) {
    throw std::runtime_error("Failed to bind number parameter");
  }
}

void HybridPreparedStatement::bindBoolean(double index, bool value) {
  std::lock_guard<std::mutex> lock(_mutex);
  if (_isClosed) {
    throw std::runtime_error("Prepared statement is closed");
  }
  idx_t idx = static_cast<idx_t>(index);
  if (duckdb_bind_boolean(_stmt, idx, value) == DuckDBError) {
    throw std::runtime_error("Failed to bind boolean parameter");
  }
}

void HybridPreparedStatement::bindBigInt(double index, int64_t value) {
  std::lock_guard<std::mutex> lock(_mutex);
  if (_isClosed) {
    throw std::runtime_error("Prepared statement is closed");
  }
  idx_t idx = static_cast<idx_t>(index);
  if (duckdb_bind_int64(_stmt, idx, value) == DuckDBError) {
    throw std::runtime_error("Failed to bind bigint parameter");
  }
}

void HybridPreparedStatement::bindBlob(
    double index, const std::shared_ptr<ArrayBuffer>& value) {
  std::lock_guard<std::mutex> lock(_mutex);
  if (_isClosed) {
    throw std::runtime_error("Prepared statement is closed");
  }
  idx_t idx = static_cast<idx_t>(index);
  if (duckdb_bind_blob(_stmt, idx, value->data(), value->size()) ==
      DuckDBError) {
    throw std::runtime_error("Failed to bind blob parameter");
  }
}

void HybridPreparedStatement::bindNull(double index) {
  std::lock_guard<std::mutex> lock(_mutex);
  if (_isClosed) {
    throw std::runtime_error("Prepared statement is closed");
  }
  idx_t idx = static_cast<idx_t>(index);
  if (duckdb_bind_null(_stmt, idx) == DuckDBError) {
    throw std::runtime_error("Failed to bind null parameter");
  }
}

QueryResult HybridPreparedStatement::resultToQueryResult(duckdb_result& result) {
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

QueryResult HybridPreparedStatement::executeInternal() {
  std::lock_guard<std::mutex> lock(_mutex);

  if (_isClosed) {
    throw std::runtime_error("Prepared statement is closed");
  }

  duckdb_result result;
  if (duckdb_execute_prepared(_stmt, &result) == DuckDBError) {
    std::string error = duckdb_result_error(&result);
    duckdb_destroy_result(&result);
    throw std::runtime_error(error);
  }

  QueryResult qr = resultToQueryResult(result);
  duckdb_destroy_result(&result);
  return qr;
}

std::shared_ptr<Promise<QueryResult>> HybridPreparedStatement::execute() {
  return Promise<QueryResult>::async([this]() {
    return executeInternal();
  });
}

QueryResult HybridPreparedStatement::executeSync() {
  return executeInternal();
}

void HybridPreparedStatement::reset() {
  std::lock_guard<std::mutex> lock(_mutex);
  if (_isClosed) {
    throw std::runtime_error("Prepared statement is closed");
  }
  duckdb_clear_bindings(_stmt);
}

void HybridPreparedStatement::close() {
  std::lock_guard<std::mutex> lock(_mutex);
  if (!_isClosed && _stmt != nullptr) {
    duckdb_destroy_prepare(&_stmt);
    _stmt = nullptr;
    _isClosed = true;
  }
}

} // namespace margelo::nitro::duckdb
