import { NitroModules } from 'react-native-nitro-modules'
import type {
  DuckDB,
  Database,
  PreparedStatement,
  QueryResult,
  DatabaseOptions,
  DuckDBValue,
} from './specs/DuckDB.nitro'

// Re-export types
export type {
  DuckDB,
  Database,
  PreparedStatement,
  QueryResult,
  DatabaseOptions,
  DuckDBValue,
}

/**
 * Parse query result rows from JSON string
 */
export interface ParsedQueryResult extends Omit<QueryResult, 'rowsJson'> {
  rows: Record<string, DuckDBValue>[]
}

/**
 * Parse a QueryResult into a more usable format with typed rows
 */
export function parseQueryResult(result: QueryResult): ParsedQueryResult {
  const rows = JSON.parse(result.rowsJson) as DuckDBValue[][]
  const { columns, rowCount, rowsAffected } = result

  // Convert array rows to objects with column names
  const objectRows = rows.map((row) => {
    const obj: Record<string, DuckDBValue> = {}
    columns.forEach((col, i) => {
      obj[col] = row[i] ?? null
    })
    return obj
  })

  return {
    columns,
    rows: objectRows,
    rowCount,
    rowsAffected,
  }
}

/**
 * Get the DuckDB module instance
 * This is a singleton that provides factory methods for creating database connections
 */
export const getDuckDB = (): DuckDB =>
  NitroModules.createHybridObject<DuckDB>('DuckDB')

// Lazy singleton instance
let _duckdb: DuckDB | null = null

/**
 * Default DuckDB module instance (lazy initialized)
 */
export const duckdb = {
  /**
   * DuckDB library version
   */
  get version(): string {
    if (!_duckdb) _duckdb = getDuckDB()
    return _duckdb.version
  },

  /**
   * Current platform
   */
  get platform(): string {
    if (!_duckdb) _duckdb = getDuckDB()
    return _duckdb.platform
  },

  /**
   * Open a database file
   * @param path Path to the database file
   * @param options Optional database configuration
   * @returns Database connection
   */
  open(path: string, options?: DatabaseOptions): Database {
    if (!_duckdb) _duckdb = getDuckDB()
    return _duckdb.open(path, options)
  },

  /**
   * Open an in-memory database
   * @param options Optional database configuration
   * @returns Database connection
   */
  openInMemory(options?: DatabaseOptions): Database {
    if (!_duckdb) _duckdb = getDuckDB()
    return _duckdb.openInMemory(options)
  },
}

export default duckdb

// Extension constants and test queries
export { EXTENSIONS, EXTENSION_TESTS, DEMO_PARQUET_URL } from './extensions'
export type { ExtensionName } from './extensions'
