include(kernel/Executable)

set(_entry_dir "${CMAKE_CURRENT_SOURCE_DIR}/${JANUS_TARGET_ARCH}")

if(NOT EXISTS "${_entry_dir}/CMakeLists.txt")
    message(FATAL_ERROR
        "Entry point not found for ${JANUS_TARGET_ARCH}.\n"
        "Expected: ${_entry_dir}")
endif()

add_subdirectory("${JANUS_TARGET_ARCH}")
