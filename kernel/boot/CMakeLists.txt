# Boot module (arch + protocol selector)

set(_tinyos_boot_impl_dir "${CMAKE_CURRENT_SOURCE_DIR}/${TINYOS_TARGET_ARCH}/${TINYOS_BOOT_PROTOCOL}")

if(EXISTS "${_tinyos_boot_impl_dir}/CMakeLists.txt")
    add_subdirectory("${TINYOS_TARGET_ARCH}/${TINYOS_BOOT_PROTOCOL}")
else()
    message(FATAL_ERROR
        "Boot implementation not found for TINYOS_TARGET_ARCH='${TINYOS_TARGET_ARCH}' and "
        "TINYOS_BOOT_PROTOCOL='${TINYOS_BOOT_PROTOCOL}'.\n"
        "Expected folder: ${_tinyos_boot_impl_dir}\n"
        "Add that folder (with a CMakeLists.txt) to support this combination.")
endif()
