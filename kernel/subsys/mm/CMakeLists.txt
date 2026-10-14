file(GLOB_RECURSE MM_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

if(JANUS_ENABLE_KTEST)
    set(_mm_ktest_sources
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/pmm_test.c"
    )
    list(APPEND JANUS_KTEST_CASE_SOURCES ${_mm_ktest_sources})
    set(JANUS_KTEST_CASE_SOURCES "${JANUS_KTEST_CASE_SOURCES}" CACHE INTERNAL "" FORCE)
endif()

janus_add_subsys(mm
    SOURCES
        ${MM_SOURCES}
    DEPENDENCIES
        kio
        mem
        janus_contract_memmap
)


