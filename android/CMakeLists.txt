project(NitroDuckdb)
cmake_minimum_required(VERSION 3.9.0)

set (PACKAGE_NAME NitroDuckdb)
set (CMAKE_VERBOSE_MAKEFILE ON)
set (CMAKE_CXX_STANDARD 20)

# DuckDB library location (pre-built)
set(DUCKDB_DIR "${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}")

# Define C++ library and add all sources
add_library(${PACKAGE_NAME} SHARED
        src/main/cpp/cpp-adapter.cpp
        ../cpp/HybridDuckDB.cpp
        ../cpp/HybridDatabase.cpp
        ../cpp/HybridPreparedStatement.cpp
)

# Add Nitrogen specs :)
include(${CMAKE_SOURCE_DIR}/../nitrogen/generated/android/NitroDuckdb+autolinking.cmake)

# Import pre-built DuckDB library
add_library(duckdb SHARED IMPORTED)
set_target_properties(duckdb PROPERTIES
        IMPORTED_LOCATION "${DUCKDB_DIR}/libduckdb.so"
)

# Set up local includes
include_directories(
        "src/main/cpp"
        "../cpp"
)

find_library(LOG_LIB log)

# Link all libraries together
target_link_libraries(
        ${PACKAGE_NAME}
        ${LOG_LIB}
        android                                   # <-- Android core
        duckdb                                    # <-- DuckDB library
)
