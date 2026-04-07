cmake_minimum_required(VERSION 3.20)

project(softadastra_wal
    VERSION 0.1.0
    DESCRIPTION "Softadastra WAL - Durable write-ahead log"
    LANGUAGES CXX
)

include(GNUInstallDirs)

# ========================
# Options
# ========================
option(SOFTADASTRA_WAL_BUILD_TESTS "Build wal tests" OFF)
option(SOFTADASTRA_WAL_BUILD_EXAMPLES "Build wal examples" OFF)

# ========================
# Global settings
# ========================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ========================
# Dependencies
# ========================
if(NOT SOFTADASTRA_UMBRELLA_BUILD)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.vix/vix_deps.cmake")
        include("${CMAKE_CURRENT_SOURCE_DIR}/.vix/vix_deps.cmake")
    endif()
endif()

if(NOT TARGET softadastra::fs AND NOT TARGET softadastra_fs)
    find_package(softadastra_fs CONFIG QUIET)
    find_package(fs CONFIG QUIET)
endif()

set(SOFTADASTRA_WAL_FS_TARGET "")
if(TARGET softadastra::fs)
    set(SOFTADASTRA_WAL_FS_TARGET softadastra::fs)
elseif(TARGET softadastra_fs)
    set(SOFTADASTRA_WAL_FS_TARGET softadastra_fs)
endif()

if(SOFTADASTRA_WAL_FS_TARGET STREQUAL "")
    message(FATAL_ERROR
        "softadastra fs target not found.\n"
        "Install dependencies with Vix:\n"
        "  vix add @softadastra/fs\n"
        "  vix install"
    )
endif()

# ========================
# Sources
# ========================
file(GLOB_RECURSE SOFTADASTRA_WAL_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# ========================
# Library
# ========================
add_library(softadastra_wal ${SOFTADASTRA_WAL_SOURCES})
add_library(softadastra::wal ALIAS softadastra_wal)

target_compile_features(softadastra_wal PUBLIC cxx_std_20)

target_link_libraries(softadastra_wal
    PUBLIC
        ${SOFTADASTRA_WAL_FS_TARGET}
)

# ========================
# Include directories
# ========================
target_include_directories(softadastra_wal
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# ========================
# Compiler options
# ========================
if(MSVC)
    target_compile_options(softadastra_wal PRIVATE /W4 /permissive-)
else()
    target_compile_options(softadastra_wal PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ========================
# Target properties
# ========================
set_target_properties(softadastra_wal PROPERTIES
    OUTPUT_NAME softadastra_wal
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
    EXPORT_NAME wal
)

# ========================
# Examples
# ========================
if(SOFTADASTRA_WAL_BUILD_EXAMPLES AND NOT SOFTADASTRA_UMBRELLA_BUILD)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/CMakeLists.txt")
        add_subdirectory(examples)
    else()
        message(STATUS "[wal] examples/ missing CMakeLists.txt -> skipping")
    endif()
endif()

# ========================
# Tests
# ========================
if(SOFTADASTRA_WAL_BUILD_TESTS)
    include(CTest)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
        add_subdirectory(tests)
    else()
        message(STATUS "[wal] tests/ missing CMakeLists.txt -> skipping")
    endif()
endif()

# ========================
# Summary
# ========================
message(STATUS "------------------------------------------------------")
message(STATUS "softadastra::wal configured (${PROJECT_VERSION})")
message(STATUS "FS target:       ${SOFTADASTRA_WAL_FS_TARGET}")
message(STATUS "Include dir:     ${CMAKE_CURRENT_SOURCE_DIR}/include")
message(STATUS "Build tests:     ${SOFTADASTRA_WAL_BUILD_TESTS}")
message(STATUS "Build examples:  ${SOFTADASTRA_WAL_BUILD_EXAMPLES}")
message(STATUS "Build type:      ${CMAKE_BUILD_TYPE}")
message(STATUS "------------------------------------------------------")
