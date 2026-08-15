#[[ 
    ArchSource.cmake - JANUS Architecture-Specific Source Helper
    Provides janus_add_arch_subsys() for building arch-specific driver implementations
]]

include_guard(GLOBAL)

#
# Add architecture-specific implementation for a subsystem or kernel library
#
# Usage:
#   janus_add_arch_subsys(<subsystem_name>
#       SOURCES file1.c file2.c ... 
#   )
#
# - Each arch declares ALL its sources explicitly
# - Shared sources referenced via relative path: ../shared/...
# - Helper exports sources back to janus_add_subsys() via global properties
# - The main subsystem library includes both portable and arch-specific code
#
# Example:
#   janus_add_arch_subsys(drivers
#       SOURCES
#           foo.c
#           bar.c
#           ../shared/base.c
#   )
#

if(NOT JANUS_PLATFORM_LOADED)
    message(FATAL_ERROR "platform/Detection.cmake must be included before ArchSource.cmake")
endif()

function(janus_add_arch_subsys NAME)
    cmake_parse_arguments(
        ARG
        ""
        ""
        "SOURCES;DEPENDENCIES"
        ${ARGN}
    )

    set(ARCH_LIB_NAME "${NAME}_arch")
    add_library(${ARCH_LIB_NAME} STATIC ${ARG_SOURCES})
    target_include_directories(${ARCH_LIB_NAME} 
    PUBLIC 
        "${CMAKE_CURRENT_SOURCE_DIR}/include"          
        "${CMAKE_CURRENT_SOURCE_DIR}/../shared/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/../include"      
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/internal"       
        "${CMAKE_CURRENT_SOURCE_DIR}/../../include" 
    )
    if(ARG_DEPENDENCIES)
        target_link_libraries(${ARCH_LIB_NAME} PUBLIC ${ARG_DEPENDENCIES})
    endif()

    janus_apply_compile_flags(${ARCH_LIB_NAME})
endfunction()
