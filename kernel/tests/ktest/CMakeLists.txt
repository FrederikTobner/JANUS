if(NOT JANUS_ENABLE_KTEST)
    return()
endif()

# The runtime is kept under the test tree and linked only when the optional
# in-kernel suite is enabled. This makes the framework explicit and isolated from
# the normal production runtime while still guaranteeing the registration section
# survives into the final kernel image.
add_library(ktest_runtime STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ktest.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ktest_exit.c"
)

target_include_directories(ktest_runtime
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_SOURCE_DIR}/kernel/include"
)
target_link_libraries(ktest_runtime
    PUBLIC
        kio
        janus_asm
)
janus_apply_compile_flags(ktest_runtime)

find_program(LUA_EXECUTABLE NAMES lua lua5.4 lua5.3 luajit)
if(NOT LUA_EXECUTABLE)
    message(WARNING "JANUS_ENABLE_KTEST=ON but no Lua interpreter found; skipping host-side ktest runner registration.")
    return()
endif()

if(JANUS_TARGET_ARCH STREQUAL "aarch64")
    find_program(QEMU_BINARY NAMES qemu-system-aarch64)
else()
    find_program(QEMU_BINARY NAMES qemu-system-x86_64)
endif()

if(NOT QEMU_BINARY)
    message(WARNING "JANUS_ENABLE_KTEST=ON but QEMU was not found; skipping host-side ktest runner registration.")
    return()
endif()

set(_ktest_dir "${CMAKE_SOURCE_DIR}/kernel/tests/ktest")
set(_runner "${_ktest_dir}/run_ktest.lua")

foreach(_proto IN LISTS JANUS_BOOT_PROTOCOLS)
    set(_iso_target "iso-${_proto}")
    set(_fixture "ktest_iso_${_proto}")

    add_test(NAME build_${_iso_target}
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ${_iso_target})
    set_tests_properties(build_${_iso_target} PROPERTIES FIXTURES_SETUP ${_fixture})

    if(_proto STREQUAL "multiboot2")
        set(_iso "${CMAKE_BINARY_DIR}/janus_${JANUS_TARGET_ARCH}_grub.iso")
    else()
        set(_iso "${CMAKE_BINARY_DIR}/janus_${JANUS_TARGET_ARCH}.iso")
    endif()

    set(_test_name "ktest_${_proto}")
    set(_machine_arg "")
    if(QEMU_MACHINE_FLAGS)
        string(REPLACE ";" " " _machine "${QEMU_MACHINE_FLAGS}")
        set(_machine_arg --machine "${_machine}")
    endif()

    set(_bios_arg "")
    if(JANUS_TARGET_ARCH STREQUAL "aarch64" AND AARCH64_UEFI_FIRMWARE)
        set(_bios_arg --bios ${AARCH64_UEFI_FIRMWARE})
    endif()

    add_test(NAME ${_test_name}
        COMMAND ${LUA_EXECUTABLE} ${_runner}
                --qemu ${QEMU_BINARY}
                --iso ${_iso}
                ${_machine_arg}
                ${_bios_arg}
                --timeout 60
                --log ${CMAKE_BINARY_DIR}/ktest-${_proto}.log)

    set_tests_properties(${_test_name} PROPERTIES
        FIXTURES_REQUIRED ${_fixture}
        RESOURCE_LOCK qemu_serial
        TIMEOUT 120
        SKIP_RETURN_CODE 77
        ENVIRONMENT "LUA_PATH=${_ktest_dir}/?.lua;;")

    message(STATUS "KTest: registered ${_test_name} (${_iso})")
endforeach()
