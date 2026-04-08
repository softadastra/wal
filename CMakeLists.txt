cmake_minimum_required(VERSION 3.20)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    project(softadastra_wal
        VERSION 0.1.0
        DESCRIPTION "Softadastra WAL - Durable write-ahead log"
        LANGUAGES CXX
    )
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# ========================
# Standalone fallback options
# ========================
if(NOT DEFINED SOFTADASTRA_WARNINGS_AS_ERRORS)
    option(SOFTADASTRA_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
endif()

if(SOFTADASTRA_UMBRELLA_BUILD)
    set(SOFTADASTRA_WAL_BUILD_TESTS ${SOFTADASTRA_BUILD_TESTS})
else()
    option(SOFTADASTRA_WAL_BUILD_TESTS "Build wal tests" OFF)
endif()

option(SOFTADASTRA_WAL_BUILD_EXAMPLES "Build wal examples" OFF)

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
    if(SOFTADASTRA_WARNINGS_AS_ERRORS)
        target_compile_options(softadastra_wal PRIVATE /WX)
    endif()
else()
    target_compile_options(softadastra_wal PRIVATE -Wall -Wextra -Wpedantic)
    if(SOFTADASTRA_WARNINGS_AS_ERRORS)
        target_compile_options(softadastra_wal PRIVATE -Werror)
    endif()
endif()

# ========================
# Target properties
# ========================
set_target_properties(softadastra_wal PROPERTIES
    OUTPUT_NAME softadastra_wal
    VERSION 0.1.0
    SOVERSION 0
    EXPORT_NAME wal
)

# ========================
# Install
# ========================
install(
    TARGETS softadastra_wal
    EXPORT softadastraWalTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    EXPORT softadastraWalTargets
    FILE softadastraWalTargets.cmake
    NAMESPACE softadastra::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/softadastra_wal"
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/softadastra_walConfigVersion.cmake"
    VERSION 0.1.0
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/softadastra_walConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/softadastra_walConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/softadastra_wal"
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/softadastra_walConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/softadastra_walConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/softadastra_wal"
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
message(STATUS "softadastra::wal configured (0.1.0)")
message(STATUS "FS target:       ${SOFTADASTRA_WAL_FS_TARGET}")
message(STATUS "Include dir:     ${CMAKE_CURRENT_SOURCE_DIR}/include")
message(STATUS "Build tests:     ${SOFTADASTRA_WAL_BUILD_TESTS}")
message(STATUS "Build examples:  ${SOFTADASTRA_WAL_BUILD_EXAMPLES}")
message(STATUS "Warnings=error:  ${SOFTADASTRA_WARNINGS_AS_ERRORS}")
message(STATUS "Build type:      ${CMAKE_BUILD_TYPE}")
message(STATUS "Install prefix:  ${CMAKE_INSTALL_PREFIX}")
message(STATUS "------------------------------------------------------")
