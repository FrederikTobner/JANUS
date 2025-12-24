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
                ${CMAKE_SOURCE_DIR}/kernel/include
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
            ${CMAKE_SOURCE_DIR}kernel/include
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

# Print TinyOS build configuration summary
#
# Create the final kernel executable with custom linker script
# 
# Usage:
#   tinyos_create_kernel(
#       SOURCES main.c init.c
#       LIBRARIES lib1 lib2
#       LINKER_SCRIPT path/to/linker.ld
#   )
#
function(tinyos_create_kernel)
    cmake_parse_arguments(
        ARG
        ""
        "LINKER_SCRIPT"
        "SOURCES;LIBRARIES"
        ${ARGN}
    )

    # Validate required arguments
    if(NOT ARG_SOURCES)
        message(FATAL_ERROR "tinyos_create_kernel: SOURCES required")
    endif()
    if(NOT ARG_LINKER_SCRIPT)
        message(FATAL_ERROR "tinyos_create_kernel: LINKER_SCRIPT required")
    endif()

    # Create kernel executable
    add_executable(kernel.elf ${ARG_SOURCES})

    # Link libraries
    if(ARG_LIBRARIES)
        target_link_libraries(kernel.elf PRIVATE ${ARG_LIBRARIES})
    endif()

    # Include directories
    target_include_directories(kernel.elf
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/kernel/include
            ${CMAKE_BINARY_DIR}/include
    )

    # Apply kernel compiler flags
    target_compile_options(kernel.elf PRIVATE ${TINYOS_COMMON_FLAGS})
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(kernel.elf PRIVATE ${TINYOS_DEBUG_FLAGS})
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(kernel.elf PRIVATE ${TINYOS_RELEASE_FLAGS})
    elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
        target_compile_options(kernel.elf PRIVATE ${TINYOS_MINSIZEREL_FLAGS})
    endif()

    # Custom linker flags with linker script
    set_target_properties(kernel.elf PROPERTIES
        LINK_FLAGS "-T ${ARG_LINKER_SCRIPT} -nostdlib -static"
        LINK_DEPENDS "${ARG_LINKER_SCRIPT}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    )

    message(STATUS "  Created kernel executable: kernel.elf")
    message(STATUS "    Linker script: ${ARG_LINKER_SCRIPT}")
    message(STATUS "    Libraries: ${ARG_LIBRARIES}")
endfunction()

