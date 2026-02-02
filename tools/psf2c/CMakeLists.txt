# Tools are built with the host compiler, not the cross-compiler
# This allows them to run on the build machine
cmake_minimum_required(VERSION 3.20)
project(PSF2C VERSION 0.1.0 LANGUAGES C)

# Print build configuration
message(STATUS "========================================")
message(STATUS "PSF2C Build Configuration")
message(STATUS "========================================")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "========================================")

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Option to enable tests
option(PSF2C_ENABLE_TESTS "Enable tests for PSF2C" ON)

# Fetch GoogleTest for tests
if(PSF2C_ENABLE_TESTS)
    # Enable C++ for tests
    enable_language(CXX)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    # Prevent gtest from overriding compiler/linker options
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
endif()

add_subdirectory(src)

# Tests
if(PSF2C_ENABLE_TESTS)
    add_subdirectory(tests)
endif()

