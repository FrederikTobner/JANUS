# TinyOS CMake Helper Functions
# Provides reusable functions for creating kernel libraries and modules

include_guard(GLOBAL)

# Ensure platform is loaded
if(NOT TINYOS_PLATFORM_LOADED)
    message(FATAL_ERROR "TinyOSPlatform.cmake must be included before TinyOSHelpers.cmake")
endif()

#
# Add a kernel library with standard configuration
# 
# Usage:
#   tinyos_add_library(name
#       SOURCES file1.c file2.c
#       [DEPENDENCIES dep1 dep2]
#   )
#
function(tinyos_add_library NAME)
    cmake_parse_arguments(
        ARG                    # Prefix for parsed arguments
        ""                     # Options (boolean flags)
        ""                     # Single-value arguments
        "SOURCES;DEPENDENCIES" # Multi-value arguments
        ${ARGN}
    )

    # Validate required arguments
    if(NOT ARG_SOURCES)
        # Allow empty SOURCES for placeholder modules
        message(STATUS "  Added library: ${NAME} (placeholder, no sources yet)")
        add_library(${NAME} INTERFACE)
        
        # Include directories for INTERFACE library
        target_include_directories(${NAME}
            INTERFACE
                ${CMAKE_CURRENT_SOURCE_DIR}/include
                ${CMAKE_SOURCE_DIR}/include
        )
        
        # Link dependencies
        if(ARG_DEPENDENCIES)
            target_link_libraries(${NAME} INTERFACE ${ARG_DEPENDENCIES})
        endif()
        
        return()
    endif()

    # Create static library
    add_library(${NAME} STATIC ${ARG_SOURCES})

    # Standard include directories
    target_include_directories(${NAME}
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/include
    )

    # Link dependencies
    if(ARG_DEPENDENCIES)
        target_link_libraries(${NAME} PUBLIC ${ARG_DEPENDENCIES})
    endif()

    # Apply common compiler flags
    target_compile_options(${NAME} PRIVATE ${TINYOS_COMMON_FLAGS})
    
    # Apply build-type specific flags
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${NAME} PRIVATE ${TINYOS_DEBUG_FLAGS})
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(${NAME} PRIVATE ${TINYOS_RELEASE_FLAGS})
    elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
        target_compile_options(${NAME} PRIVATE ${TINYOS_MINSIZEREL_FLAGS})
    endif()

    message(STATUS "  Added library: ${NAME}")
endfunction()

#
# Add a kernel module (like kernel, boot, arch, mm)
# 
# Usage:
#   tinyos_add_module(name
#       SOURCES file1.c file2.c
#       [DEPENDENCIES dep1 dep2]
#   )
#
function(tinyos_add_module NAME)
    cmake_parse_arguments(
        ARG
        ""
        ""
        "SOURCES;DEPENDENCIES"
        ${ARGN}
    )

    # Validate required arguments
    if(NOT ARG_SOURCES)
        # Allow empty SOURCES for placeholder modules
        message(STATUS "  Added module: ${NAME} (placeholder, no sources yet)")
        add_library(${NAME} INTERFACE)
        
        # Include directories for INTERFACE library
        target_include_directories(${NAME}
            INTERFACE
                ${CMAKE_CURRENT_SOURCE_DIR}/include
                ${CMAKE_SOURCE_DIR}/include
        )
        
        # Link dependencies
        if(ARG_DEPENDENCIES)
            target_link_libraries(${NAME} INTERFACE ${ARG_DEPENDENCIES})
        endif()
        
        return()
    endif()

    # Create static library (modules are libraries)
    add_library(${NAME} STATIC ${ARG_SOURCES})

    # Standard include directories
    target_include_directories(${NAME}
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/include
    )

    # Link dependencies
    if(ARG_DEPENDENCIES)
        target_link_libraries(${NAME} PUBLIC ${ARG_DEPENDENCIES})
    endif()

    # Apply common compiler flags
    target_compile_options(${NAME} PRIVATE ${TINYOS_COMMON_FLAGS})
    
    # Apply build-type specific flags
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${NAME} PRIVATE ${TINYOS_DEBUG_FLAGS})
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(${NAME} PRIVATE ${TINYOS_RELEASE_FLAGS})
    elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
        target_compile_options(${NAME} PRIVATE ${TINYOS_MINSIZEREL_FLAGS})
    endif()

    message(STATUS "  Added module: ${NAME}")
endfunction()

#
# Add assembly sources to a target
# 
# Usage:
#   tinyos_add_asm(target file1.asm file2.asm)
#
function(tinyos_add_asm TARGET)
    foreach(ASM_FILE ${ARGN})
        # Get absolute path
        get_filename_component(ASM_ABS ${ASM_FILE} ABSOLUTE)
        
        # Get output object file name
        get_filename_component(ASM_NAME ${ASM_FILE} NAME_WE)
        set(ASM_OBJ ${CMAKE_CURRENT_BINARY_DIR}/${ASM_NAME}.o)

        # Add custom command to assemble with NASM
        add_custom_command(
            OUTPUT ${ASM_OBJ}
            COMMAND nasm -f elf64 -o ${ASM_OBJ} ${ASM_ABS}
            DEPENDS ${ASM_ABS}
            COMMENT "Assembling ${ASM_FILE}"
        )

        # Add object file to target
        target_sources(${TARGET} PRIVATE ${ASM_OBJ})
    endforeach()
    
    message(STATUS "  Added assembly sources to ${TARGET}: ${ARGN}")
endfunction()

#
# Add BMUnit tests for a module
# 
# Usage:
#   tinyos_add_tests(module_name
#       SOURCES test1.c test2.c
#       [DEPENDENCIES dep1 dep2]
#   )
#
function(tinyos_add_tests MODULE_NAME)
    if(NOT BUILD_TESTS)
        return()
    endif()

    cmake_parse_arguments(
        ARG
        ""
        ""
        "SOURCES;DEPENDENCIES"
        ${ARGN}
    )

    if(NOT ARG_SOURCES)
        return()
    endif()

    foreach(TEST_FILE ${ARG_SOURCES})
        # Get test name from filename
        get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
        set(TEST_TARGET "${MODULE_NAME}_${TEST_NAME}")

        # Create test executable
        add_executable(${TEST_TARGET} ${TEST_FILE})

        # Link with module and dependencies
        target_link_libraries(${TEST_TARGET}
            PRIVATE
                ${MODULE_NAME}
                bmunit
                ${ARG_DEPENDENCIES}
        )

        # Include directories
        target_include_directories(${TEST_TARGET}
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/include
                ${CMAKE_SOURCE_DIR}/include
        )

        # Compiler flags
        target_compile_options(${TEST_TARGET} PRIVATE ${TINYOS_COMMON_FLAGS})
        
        # Register with CTest
        add_test(NAME ${TEST_TARGET} COMMAND ${TEST_TARGET})
        
        message(STATUS "  Added test: ${TEST_TARGET}")
    endforeach()
endfunction()

#
# Print TinyOS build configuration summary
#
function(tinyos_print_config)
    message(STATUS "")
    message(STATUS "═══════════════════════════════════════")
    message(STATUS "    TinyOS Build Configuration")
    message(STATUS "═══════════════════════════════════════")
    message(STATUS "  Build type:    ${CMAKE_BUILD_TYPE}")
    message(STATUS "  Host:          ${CMAKE_SYSTEM_NAME}")
    message(STATUS "  Target:        ${TINYOS_TARGET_ARCH}-${TINYOS_TARGET_PLATFORM}")
    message(STATUS "  C Compiler:    ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
    message(STATUS "  Assembler:     nasm")
    message(STATUS "  Build tests:   ${BUILD_TESTS}")
    message(STATUS "═══════════════════════════════════════")
    message(STATUS "")
endfunction()
