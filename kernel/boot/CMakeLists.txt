# Boot module (arch + protocol selector)

set(_janus_boot_impl_dir "${CMAKE_CURRENT_SOURCE_DIR}/${JANUS_TARGET_ARCH}/${JANUS_BOOT_PROTOCOL}")

if(EXISTS "${_janus_boot_impl_dir}/CMakeLists.txt")
    add_subdirectory("${JANUS_TARGET_ARCH}/${JANUS_BOOT_PROTOCOL}")
else()
    message(FATAL_ERROR
        "Boot implementation not found for JANUS_TARGET_ARCH='${JANUS_TARGET_ARCH}' and "
        "JANUS_BOOT_PROTOCOL='${JANUS_BOOT_PROTOCOL}'.\n"
        "Expected folder: ${_janus_boot_impl_dir}\n"
        "Add that folder (with a CMakeLists.txt) to support this combination.")
endif()
