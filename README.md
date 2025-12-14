# @walkthru-earth/react-native-duckdb

High-performance native DuckDB bindings for React Native using [Nitro Modules](https://github.com/mrousavy/nitro).

## Features

- Native DuckDB integration (not WASM)
- High performance via JSI bindings
- Full SQL support including:
  - SELECT, INSERT, UPDATE, DELETE
  - Prepared statements with parameter binding
  - Transactions
  - Extensions (parquet, json, httpfs, etc.)
- Supports iOS and Android
- TypeScript first API

## Requirements

| Platform     | Minimum Version          |
| ------------ | ------------------------ |
| iOS          | 13.4+                    |
| Android      | API 24+ (Android 7.0)    |
| React Native | 0.76+ (New Architecture) |

### Supported Architectures

| Platform | Architectures                                    |
| -------- | ------------------------------------------------ |
| iOS      | `arm64` (device), `arm64` + `x86_64` (simulator) |
| Android  | `arm64-v8a`, `armeabi-v7a`, `x86_64`             |

## Installation

```bash
npm install @walkthru-earth/react-native-duckdb react-native-nitro-modules
# or
yarn add @walkthru-earth/react-native-duckdb react-native-nitro-modules
# or
bun add @walkthru-earth/react-native-duckdb react-native-nitro-modules
```

### iOS

```bash
cd ios && pod install
```

### Android

No additional setup required. The package includes pre-built native libraries for the following architectures:

- `arm64-v8a` (64-bit ARM, most modern devices)
- `armeabi-v7a` (32-bit ARM, older devices)
- `x86_64` (64-bit x86, emulators)

Your app's `android/app/build.gradle` should include the architectures you want to support:

```gradle
android {
    defaultConfig {
        ndk {
            abiFilters "armeabi-v7a", "x86_64", "arm64-v8a"
        }
    }
}
```

## Usage

### Basic Usage

```typescript
import { duckdb, parseQueryResult } from 'react-native-duckdb'

// Open an in-memory database
const db = duckdb.openInMemory()

// Execute SQL
await db.execute('CREATE TABLE users (id INTEGER, name VARCHAR)')
await db.execute("INSERT INTO users VALUES (1, 'Alice'), (2, 'Bob')")

// Query data
const result = await db.execute('SELECT * FROM users')
const { rows } = parseQueryResult(result)
console.log(rows) // [{ id: 1, name: 'Alice' }, { id: 2, name: 'Bob' }]

// Close when done
db.close()
```

### File-based Database

```typescript
import { duckdb } from 'react-native-duckdb'
import { DocumentDirectoryPath } from 'react-native-fs'

const dbPath = `${DocumentDirectoryPath}/mydata.duckdb`
const db = duckdb.open(dbPath)

// Your database will persist between app restarts
```

### Prepared Statements

```typescript
const stmt = db.prepare('SELECT * FROM users WHERE id = ?')
stmt.bindNumber(1, 1)
const result = await stmt.execute()

// Reuse with different parameters
stmt.reset()
stmt.bindNumber(1, 2)
const result2 = await stmt.execute()

stmt.close()
```

### Transactions

```typescript
await db.beginTransaction()
try {
  await db.execute('INSERT INTO users VALUES (3, "Charlie")')
  await db.execute('UPDATE stats SET user_count = user_count + 1')
  await db.commit()
} catch (error) {
  await db.rollback()
  throw error
}
```

### Load Extensions

```typescript
// Load a pre-built extension
await db.loadExtension('parquet')

// Query parquet files directly
const result = await db.execute("SELECT * FROM 'data.parquet'")
```

### Database Options

```typescript
const db = duckdb.open('/path/to/db.duckdb', {
  readOnly: true,
  threads: 4,
  maxMemory: '2GB',
})
```

## API Reference

### `duckdb`

Main module with factory methods:

- `duckdb.version` - DuckDB library version
- `duckdb.platform` - Current platform ('ios' or 'android')
- `duckdb.open(path, options?)` - Open a file-based database
- `duckdb.openInMemory(options?)` - Open an in-memory database

### `Database`

Database connection methods:

- `execute(sql)` - Execute SQL asynchronously
- `executeSync(sql)` - Execute SQL synchronously
- `prepare(sql)` - Create a prepared statement
- `beginTransaction()` / `commit()` / `rollback()` - Transaction control
- `loadExtension(name)` / `installExtension(name)` - Extension management
- `close()` - Close the connection

### `PreparedStatement`

Prepared statement methods:

- `bindString(index, value)` - Bind a string parameter
- `bindNumber(index, value)` - Bind a number parameter
- `bindBoolean(index, value)` - Bind a boolean parameter
- `bindBigInt(index, value)` - Bind a bigint parameter
- `bindBlob(index, value)` - Bind an ArrayBuffer parameter
- `bindNull(index)` - Bind null
- `execute()` / `executeSync()` - Execute the statement
- `reset()` - Clear bindings for reuse
- `close()` - Deallocate the statement

### `parseQueryResult(result)`

Helper function to parse `QueryResult` into a more usable format with typed row objects.

### Extension Constants

```typescript
import {
  EXTENSIONS,
  EXTENSION_TESTS,
  DEMO_PARQUET_URL,
} from 'react-native-duckdb'

// Available extension names
console.log(EXTENSIONS.json) // 'json'
console.log(EXTENSIONS.parquet) // 'parquet'

// Test queries for E2E testing
console.log(EXTENSION_TESTS.json) // SELECT json_extract...

// Demo parquet file for httpfs testing
console.log(DEMO_PARQUET_URL) // https://duckdb.org/data/holdings.parquet
```

## Pre-built Extensions

The following extensions are pre-built and included:

| Extension          | Description                          | Android | iOS |
| ------------------ | ------------------------------------ | :-----: | :-: |
| `icu`              | International Components for Unicode |   ✅    | ✅  |
| `json`             | JSON functions                       |   ✅    | ✅  |
| `parquet`          | Parquet file support                 |   ✅    | ✅  |
| `httpfs`           | HTTP/S3 file system                  |   ✅    | ✅  |
| `fts`              | Full-text search                     |   ✅    | ✅  |
| `inet`             | Network address functions            |   ✅    | ✅  |
| `vss`              | Vector similarity search             |   ✅    | ✅  |
| `autocomplete`     | Query autocomplete                   |   ✅    | ✅  |
| `sqlite_scanner`   | SQLite file scanner                  |   ✅    | ✅  |
| `h3`               | H3 hexagonal geospatial indexing     |   ✅    | ✅  |
| `postgres_scanner` | PostgreSQL connector                 |   ❌    | ✅  |

## Development

### Building from Source

This project uses pre-built DuckDB libraries from [duckdb-dart](https://github.com/yharby/duckdb-dart).

```bash
# Install dependencies
bun install

# Generate Nitro specs
bun run specs

# TypeCheck
bun run typecheck
```

### Testing

```bash
# Run unit tests
bun run test

# Run tests with coverage
bun run test:coverage
```

### Pre-commit Hooks

This project uses Husky + lint-staged for pre-commit hooks:

- ESLint + Prettier on staged TypeScript files
- All tests run before each commit

## License

MIT

## Credits

- [DuckDB](https://duckdb.org) - The database engine
- [Nitro Modules](https://github.com/mrousavy/nitro) - High-performance React Native bindings
- [duckdb-dart](https://github.com/yharby/duckdb-dart) - DuckDB Dart bindings with pre-built native library workflows
