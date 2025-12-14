/**
 * DuckDB Extensions available in react-native-duckdb
 *
 * Pre-built extensions from duckdb-dart for Android and iOS.
 * @see https://github.com/yharby/duckdb-dart
 */

/**
 * Available extensions in this build
 */
export const EXTENSIONS = {
  // Core (both platforms)
  icu: 'icu', // Unicode collation & timezone
  json: 'json', // JSON functions
  parquet: 'parquet', // Parquet files
  httpfs: 'httpfs', // HTTP/S3 access
  fts: 'fts', // Full-text search
  inet: 'inet', // IP address handling
  vss: 'vss', // Vector similarity search
  autocomplete: 'autocomplete', // SQL autocomplete
  sqlite_scanner: 'sqlite', // SQLite file access
  // Community
  h3: 'h3', // H3 geospatial indexing
  // iOS only
  postgres_scanner: 'postgres', // PostgreSQL (iOS only)
} as const

export type ExtensionName = (typeof EXTENSIONS)[keyof typeof EXTENSIONS]

/**
 * Simple smoke test queries for E2E tests
 * Each query should return a truthy result if extension works
 */
export const EXTENSION_TESTS = {
  /** JSON: extract value from JSON */
  json: `SELECT json_extract('{"a":1}', '$.a') AS result;`,

  /** ICU: case-insensitive comparison */
  icu: `SELECT 'TEST' = 'test' COLLATE NOCASE AS result;`,

  /** INET: parse IP address */
  inet: `SELECT host('192.168.1.1'::INET) AS result;`,

  /** VSS: vector distance (returns 0 for identical vectors) */
  vss: `SELECT array_distance([1.0,2.0,3.0]::FLOAT[3], [1.0,2.0,3.0]::FLOAT[3]) AS result;`,

  /** H3: lat/lng to cell ID */
  h3: `SELECT h3_latlng_to_cell(37.7749, -122.4194, 9) AS result;`,

  /** Autocomplete: SQL suggestions */
  autocomplete: `SELECT suggestion AS result FROM sql_auto_complete('SEL') LIMIT 1;`,
} as const
