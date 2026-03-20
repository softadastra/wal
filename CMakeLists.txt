cmake_minimum_required(VERSION 3.20)

project(softadastra_wal
    VERSION 0.1.0
    DESCRIPTION "Softadastra WAL - Durable write-ahead log"
    LANGUAGES CXX
)

# ========================
# Options
# ========================
option(SOFTADASTRA_WAL_BUILD_TESTS "Build wal tests" ON)
option(SOFTADASTRA_WAL_BUILD_EXAMPLES "Build wal examples" ON)

# ========================
# Library
# ========================
add_library(softadastra_wal)

# C++ standard
target_compile_features(softadastra_wal PUBLIC cxx_std_20)

# ========================
# Dependencies
# ========================
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.vix/vix_deps.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/.vix/vix_deps.cmake)
endif()

if(NOT TARGET softadastra::fs)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../fs/CMakeLists.txt)
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../fs build_fs)
    else()
        message(FATAL_ERROR
            "softadastra::fs not found. "
            "Install dependencies with Vix or provide ../fs in the monorepo."
        )
    endif()
endif()

target_link_libraries(softadastra_wal
    PUBLIC
        softadastra::fs
)

# ========================
# Include directories
# ========================
target_include_directories(softadastra_wal
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# ========================
# Sources
# ========================
file(GLOB_RECURSE WAL_SOURCES
    CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

target_sources(softadastra_wal
    PRIVATE
        ${WAL_SOURCES}
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
# Position Independent Code
# ========================
set_target_properties(softadastra_wal PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# ========================
# Namespacing
# ========================
add_library(softadastra::wal ALIAS softadastra_wal)

# ========================
# Examples
# ========================
if(SOFTADASTRA_WAL_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ========================
# Tests
# ========================
if(SOFTADASTRA_WAL_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
