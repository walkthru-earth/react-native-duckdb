import type { HybridObject } from 'react-native-nitro-modules'

/**
 * A value that can be stored in a DuckDB column
 * Nitro supports: string, number, boolean, bigint, ArrayBuffer, null
 */
export type DuckDBValue =
  | string
  | number
  | boolean
  | bigint
  | ArrayBuffer
  | null

/**
 * Result of a DuckDB query execution
 * Rows are returned as arrays of arrays (column-major format for performance)
 */
export interface QueryResult {
  /** Column names in the result set */
  columns: string[]
  /** Row data as JSON string (to be parsed on JS side) */
  rowsJson: string
  /** Number of rows returned */
  rowCount: number
  /** Number of rows affected (for INSERT/UPDATE/DELETE) */
  rowsAffected: number
}

/**
 * Options for opening a DuckDB database
 */
export interface DatabaseOptions {
  /** Enable read-only mode */
  readOnly?: boolean
  /** Number of threads for query execution (0 = auto) */
  threads?: number
  /** Maximum memory limit (e.g., "2GB") */
  maxMemory?: string
  /** Access mode: 'automatic', 'read_only', 'read_write' */
  accessMode?: string
}

/**
 * Prepared statement for parameterized queries
 */
export interface PreparedStatement extends HybridObject<{
  ios: 'c++'
  android: 'c++'
}> {
  /**
   * Bind a string parameter
   * @param index Parameter index (1-based)
   * @param value String value
   */
  bindString(index: number, value: string): void

  /**
   * Bind a number parameter
   * @param index Parameter index (1-based)
   * @param value Number value
   */
  bindNumber(index: number, value: number): void

  /**
   * Bind a boolean parameter
   * @param index Parameter index (1-based)
   * @param value Boolean value
   */
  bindBoolean(index: number, value: boolean): void

  /**
   * Bind a bigint parameter
   * @param index Parameter index (1-based)
   * @param value BigInt value
   */
  bindBigInt(index: number, value: bigint): void

  /**
   * Bind a blob parameter
   * @param index Parameter index (1-based)
   * @param value ArrayBuffer value
   */
  bindBlob(index: number, value: ArrayBuffer): void

  /**
   * Bind a null parameter
   * @param index Parameter index (1-based)
   */
  bindNull(index: number): void

  /**
   * Execute the prepared statement
   * @returns Query result
   */
  execute(): Promise<QueryResult>

  /**
   * Execute the prepared statement synchronously
   * @returns Query result
   */
  executeSync(): QueryResult

  /**
   * Reset the statement for re-execution with new parameters
   */
  reset(): void

  /**
   * Close and deallocate the prepared statement
   */
  close(): void
}

/**
 * DuckDB database connection
 */
export interface Database extends HybridObject<{ ios: 'c++'; android: 'c++' }> {
  /** Path to the database file (empty for in-memory) */
  readonly path: string

  /** Whether the database connection is open */
  readonly isOpen: boolean

  /** Whether the database is read-only */
  readonly isReadOnly: boolean

  /**
   * Execute a SQL query asynchronously
   * @param sql SQL query string
   * @returns Query result
   */
  execute(sql: string): Promise<QueryResult>

  /**
   * Execute a SQL query synchronously (blocks JS thread)
   * @param sql SQL query string
   * @returns Query result
   */
  executeSync(sql: string): QueryResult

  /**
   * Prepare a SQL statement for repeated execution
   * @param sql SQL query string with ? or $1 placeholders
   * @returns Prepared statement object
   */
  prepare(sql: string): PreparedStatement

  /**
   * Begin a new transaction
   */
  beginTransaction(): Promise<void>

  /**
   * Commit the current transaction
   */
  commit(): Promise<void>

  /**
   * Rollback the current transaction
   */
  rollback(): Promise<void>

  /**
   * Load a DuckDB extension
   * @param extensionName Name of the extension to load
   */
  loadExtension(extensionName: string): Promise<void>

  /**
   * Install a DuckDB extension
   * @param extensionName Name of the extension to install
   */
  installExtension(extensionName: string): Promise<void>

  /**
   * Close the database connection
   */
  close(): void
}

/**
 * Main DuckDB module interface
 * Factory for creating database connections
 */
export interface DuckDB extends HybridObject<{ ios: 'c++'; android: 'c++' }> {
  /** DuckDB library version */
  readonly version: string

  /** Current platform ('ios' or 'android') */
  readonly platform: string

  /**
   * Open a database file
   * @param path Path to the database file
   * @param options Optional database configuration
   * @returns Database connection
   */
  open(path: string, options?: DatabaseOptions): Database

  /**
   * Open an in-memory database
   * @param options Optional database configuration
   * @returns Database connection
   */
  openInMemory(options?: DatabaseOptions): Database
}
