#[[
    SmokeTests.cmake - JANUS Smoke-Test Registration

    Provides janus_register_smoke_tests(), which registers CTest smoke tests that
    boot the built ISO in QEMU headless and assert on serial output.
    These are HOST-SIDE tests: a Lua runner (kernel/tests/smoke/run_smoke.lua) plus CTest glue. 
    Nothing here is compiled into a kernel artifact, so the tests never enter the kernel's 
    layered dependency graph even though they live under kernel/.
]]

include_guard(GLOBAL)

# Off by default since only developers will want to run these tests 
option(JANUS_ENABLE_SMOKE_TESTS "Register QEMU serial smoke tests with CTest" OFF)

function(janus_register_smoke_tests)
    if(NOT JANUS_ENABLE_SMOKE_TESTS)
        return()
    endif()

    find_program(LUA_EXECUTABLE NAMES lua lua5.4 lua5.3 luajit)
    if(NOT LUA_EXECUTABLE)
        message(WARNING "JANUS_ENABLE_SMOKE_TESTS=ON but no Lua interpreter found; skipping smoke tests.")
        return()
    endif()
    if(NOT JANUS_QEMU_FOUND)
        message(STATUS "Smoke tests enabled but QEMU not found; smoke tests will report as skipped.")
    endif()

    set(_smoke_dir "${CMAKE_SOURCE_DIR}/kernel/tests/smoke")
    set(_runner "${_smoke_dir}/run_smoke.lua")

    # QEMU_MACHINE_FLAGS is a CMake list (";"-separated). Flatten it to a single
    # space-separated shell fragment so it survives as one --machine argument and
    # the runner can splice it into the QEMU command line verbatim.
    string(REPLACE ";" " " _machine "${QEMU_MACHINE_FLAGS}")

    if(JANUS_TEST_FAULTS)
        set(_profile "fault")
    elseif(JANUS_TEST_KMALLOC)
        set(_profile "kmalloc")
    else()
        set(_profile "nominal")
    endif()

    foreach(_proto IN LISTS JANUS_BOOT_PROTOCOLS)
        if(NOT JANUS_XORRISO_FOUND)
            message(STATUS "Smoke: xorriso missing; skipping '${_proto}' smoke test.")
            continue()
        endif()
        if(_proto STREQUAL "multiboot2" AND NOT JANUS_GRUB_MKRESCUE_FOUND)
            message(STATUS "Smoke: grub-mkrescue missing; skipping 'multiboot2' smoke test.")
            continue()
        endif()

        set(_iso_target "iso-${_proto}")
        set(_fixture "smoke_iso_${_proto}")

        add_test(NAME build_${_iso_target}
            COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ${_iso_target})
        set_tests_properties(build_${_iso_target} PROPERTIES FIXTURES_SETUP ${_fixture})

        if(_proto STREQUAL "multiboot2")
            set(_iso "${CMAKE_BINARY_DIR}/janus_${JANUS_TARGET_ARCH}_grub.iso")
        else()
            set(_iso "${CMAKE_BINARY_DIR}/${JANUS_ISO_NAME}")
        endif()

        set(_bios_arg "")
        if(JANUS_TARGET_ARCH STREQUAL "aarch64" AND AARCH64_UEFI_FIRMWARE)
            set(_bios_arg --bios ${AARCH64_UEFI_FIRMWARE})
        endif()

        set(_test_name "smoke_${_profile}_${_proto}")
        add_test(NAME ${_test_name}
            COMMAND ${LUA_EXECUTABLE} ${_runner}
                    --qemu ${QEMU_BINARY}
                    --iso ${_iso}
                    --profile ${_profile}
                    --machine "${_machine}"
                    ${_bios_arg}
                    --timeout 30
                    --log ${CMAKE_BINARY_DIR}/smoke-${_profile}-${_proto}.log)

        set_tests_properties(${_test_name} PROPERTIES
            FIXTURES_REQUIRED ${_fixture}
            RESOURCE_LOCK qemu_serial          # serialise QEMU runs under `ctest -j`
            TIMEOUT 90                         # backstop above the runner's own deadline
            SKIP_RETURN_CODE 77                # runner exits 77 when QEMU/firmware absent
            ENVIRONMENT "LUA_PATH=${_smoke_dir}/?.lua;;")

        message(STATUS "Smoke: registered ${_test_name} (${_iso})")
    endforeach()
endfunction()
