# Build Instructions

This document covers how to build `react-native-duckdb` from source with the latest toolchain (Gradle 9 + Java 25).

## Prerequisites

- **Node.js** 20+
- **Bun** (or npm/yarn)
- **Java 25** (OpenJDK)
- **Gradle 9.0** (via wrapper)
- **Android SDK** with NDK 27.1+
- **Xcode 16+** (for iOS)

## Quick Start

```bash
# Install dependencies
bun install

# Generate Nitro specs
bun run specs

# TypeScript check
bun run typecheck
```

## Android Build

### Known Issues & Fixes

#### 1. Gradle 9 + foojay-resolver-convention 0.5.0 Incompatibility

React Native 0.83's `@react-native/gradle-plugin` uses `foojay-resolver-convention` version 0.5.0, which references `JvmVendorSpec.IBM_SEMERU` - a field removed in Gradle 9.

**Fix:** Patch the gradle plugin after npm install:

```bash
sed -i '' 's/version("0.5.0")/version("1.0.0")/g' \
  node_modules/@react-native/gradle-plugin/settings.gradle.kts
```

Or add to your `settings.gradle`:

```groovy
plugins {
    id("com.facebook.react.settings")
    id("org.gradle.toolchains.foojay-resolver-convention") version "1.0.0"
}
```

#### 2. Java 25 Native Access Warning

Java 25 restricts native library access. Add to `gradle.properties`:

```properties
org.gradle.jvmargs=-Xmx2048m -XX:MaxMetaspaceSize=512m --enable-native-access=ALL-UNNAMED
```

Or set environment variable before building:

```bash
export _JAVA_OPTIONS="--enable-native-access=ALL-UNNAMED"
```

#### 3. x86 Architecture Not Supported

DuckDB pre-built libraries don't include x86 (32-bit Intel). Exclude it in `gradle.properties`:

```properties
reactNativeArchitectures=armeabi-v7a,arm64-v8a,x86_64
```

### Building the Example App

```bash
cd example/DuckDBExample

# Install dependencies
npm install

# Install react-native-nitro-modules
npm install react-native-nitro-modules

# Install the local react-native-duckdb package
npm install ../../

# Patch foojay for Gradle 9 compatibility
sed -i '' 's/version("0.5.0")/version("1.0.0")/g' \
  node_modules/@react-native/gradle-plugin/settings.gradle.kts

# Set Android SDK path
echo "sdk.dir=$HOME/Library/Android/sdk" > android/local.properties

# Build
cd android
export _JAVA_OPTIONS="--enable-native-access=ALL-UNNAMED"
./gradlew assembleDebug
```

### Build Output

The APK will be at:
```
android/app/build/outputs/apk/debug/app-debug.apk
```

## iOS Build

### Prerequisites

- Xcode 16+
- CocoaPods

### Building

```bash
cd example/DuckDBExample

# Install dependencies
npm install

# Install pods
cd ios && pod install && cd ..

# Build with Xcode
xcodebuild -workspace ios/DuckDBExample.xcworkspace \
  -scheme DuckDBExample \
  -sdk iphonesimulator \
  -configuration Debug \
  -destination 'platform=iOS Simulator,name=iPhone 16' \
  build
```

## Pre-built Native Libraries

This package requires pre-built DuckDB native libraries:

### Android

Place `libduckdb.so` files in:
```
android/src/main/jniLibs/
├── arm64-v8a/libduckdb.so
├── armeabi-v7a/libduckdb.so
└── x86_64/libduckdb.so
```

### iOS

Place `DuckDB.xcframework` in:
```
ios/DuckDB.xcframework/
├── ios-arm64/
│   └── duckdb.framework/
├── ios-arm64_x86_64-simulator/
│   └── duckdb.framework/
└── Info.plist
```

### Creating XCFramework from Frameworks

If you have separate device and simulator frameworks:

```bash
xcodebuild -create-xcframework \
  -framework path/to/device/duckdb.framework \
  -framework path/to/simulator/duckdb.framework \
  -output ios/DuckDB.xcframework
```

## Nitrogen Code Generation

After modifying TypeScript interfaces in `src/specs/DuckDB.nitro.ts`:

```bash
bun run specs
```

This generates:
- `nitrogen/generated/shared/c++/` - C++ specs
- `nitrogen/generated/android/` - Android autolinking
- `nitrogen/generated/ios/` - iOS autolinking

## Autolinking Configuration

Only `DuckDB` (the factory) should be autolinked in `nitro.json`:

```json
{
  "autolinking": {
    "DuckDB": {
      "cpp": "HybridDuckDB"
    }
  }
}
```

`Database` and `PreparedStatement` are created by the factory and don't need autolinking (they don't have default constructors).

## Troubleshooting

### CMake can't find libduckdb.so

Ensure the `.so` files are in the correct jniLibs directories with the correct ABI names.

### "HybridObject is not default-constructible"

Only autolink HybridObjects that have default constructors (like factories). Remove non-factory objects from `nitro.json` autolinking.

### Build fails with "restricted method" warning

Java 25 requires `--enable-native-access=ALL-UNNAMED`. Add it to gradle.properties or set `_JAVA_OPTIONS` env var.

## References

- [Nitro Modules Documentation](https://nitro.margelo.com)
- [Gradle 9 Release Notes](https://docs.gradle.org/9.0.0/release-notes.html)
- [React Native New Architecture](https://reactnative.dev/docs/the-new-architecture/pillars-turbomodules)
- [DuckDB C API](https://duckdb.org/docs/api/c/overview)
