cmake_minimum_required(VERSION 3.20)
project(JanusTools VERSION 0.1.0 LANGUAGES C)

message(STATUS "========================================")
message(STATUS "Janus Tools Build Configuration")
message(STATUS "========================================")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "========================================")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Registry)
include(ToolLib)
include(Tool)

option(TOOLS_ENABLE_TESTS "Enable tests for all tools" ON)

if(TOOLS_ENABLE_TESTS)
    enable_language(CXX)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
endif()

add_subdirectory(lib/clap)
add_subdirectory(psf2c)

tools_write_mermaid_diagram("${CMAKE_CURRENT_SOURCE_DIR}/../docs/src/generated/deps-tools.md")
