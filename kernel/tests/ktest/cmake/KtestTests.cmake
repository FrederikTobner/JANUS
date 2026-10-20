#[[
    KtestTests.cmake - JANUS In-Kernel Test Runner Registration

    Provides janus_register_ktest_tests(), which registers CTest cases that boot
    the built ISO in QEMU headless and check the ktest summary on the serial
    console. HOST-SIDE only: a Lua runner (kernel/tests/ktest/run_ktest.lua)
    plus CTest glue; nothing here is compiled into the kernel.

    Must be called from janus_add_targets(): the QEMU settings it reuses
    (QEMU_BINARY, QEMU_MACHINE_FLAGS, AARCH64_UEFI_FIRMWARE, JANUS_ISO_NAME) are
    only visible in that call chain.
]]

include_guard(GLOBAL)

function(janus_register_ktest_tests)
    if(NOT JANUS_ENABLE_KTEST)
        return()
    endif()

    find_program(LUA_EXECUTABLE NAMES lua lua5.4 lua5.3 luajit)
    if(NOT LUA_EXECUTABLE)
        message(WARNING "JANUS_ENABLE_KTEST=ON but no Lua interpreter found; skipping ktest runner registration.")
        return()
    endif()
    if(NOT JANUS_QEMU_FOUND)
        message(STATUS "KTest enabled but QEMU not found; ktest tests will report as skipped.")
    endif()

    set(_ktest_dir "${CMAKE_SOURCE_DIR}/kernel/tests/ktest")
    set(_runner "${_ktest_dir}/run_ktest.lua")

    # QEMU_MACHINE_FLAGS is a CMake list; flatten it to one --machine argument.
    string(REPLACE ";" " " _machine "${QEMU_MACHINE_FLAGS}")

    foreach(_proto IN LISTS JANUS_BOOT_PROTOCOLS)
        if(NOT JANUS_XORRISO_FOUND)
            message(STATUS "KTest: xorriso missing; skipping '${_proto}' ktest.")
            continue()
        endif()
        if(_proto STREQUAL "multiboot2" AND NOT JANUS_GRUB_MKRESCUE_FOUND)
            message(STATUS "KTest: grub-mkrescue missing; skipping 'multiboot2' ktest.")
            continue()
        endif()

        set(_iso_target "iso-${_proto}")
        set(_fixture "ktest_iso_${_proto}")

        add_test(NAME build_${_iso_target}
            COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ${_iso_target})
        set_tests_properties(build_${_iso_target} PROPERTIES FIXTURES_SETUP ${_fixture})

        if(_proto STREQUAL "multiboot2")
            set(_iso "${CMAKE_BINARY_DIR}/janus_${JANUS_TARGET_ARCH}_grub.iso")
        else()
            set(_iso "${CMAKE_BINARY_DIR}/${JANUS_ISO_NAME}")
        endif()

        set(_machine_arg "")
        if(_machine)
            set(_machine_arg --machine "${_machine}")
        endif()

        set(_bios_arg "")
        if(JANUS_TARGET_ARCH STREQUAL "aarch64" AND AARCH64_UEFI_FIRMWARE)
            set(_bios_arg --bios ${AARCH64_UEFI_FIRMWARE})
        endif()

        set(_test_name "ktest_${_proto}")
        set(_expect_arg "")
        if(JANUS_KTEST_NEGATIVE_CONTROL)
            set(_test_name "ktest_control_${_proto}")
            set(_expect_arg --expect-fail ktest_control.deliberate_failure)
        endif()
        add_test(NAME ${_test_name}
            COMMAND ${LUA_EXECUTABLE} ${_runner}
                    --qemu ${QEMU_BINARY}
                    --iso ${_iso}
                    ${_machine_arg}
                    ${_bios_arg}
                    ${_expect_arg}
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
endfunction()
