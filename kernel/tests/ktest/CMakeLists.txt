list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Host-side runner registration; invoked later from janus_add_targets() once the
# QEMU settings are known (see KtestTests.cmake).
include(KtestTests)

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
