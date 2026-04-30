tools_add_lib(clap
    SOURCES
        src/clap.c
        src/help.c
)

option(CLAP_ENABLE_TESTS "Enable tests for clap" ${TOOLS_ENABLE_TESTS})

if(CLAP_ENABLE_TESTS)
    add_subdirectory(tests)
endif()
