option(PSF2C_ENABLE_TESTS "Enable tests for PSF2C" ${JANUS_TOOLS_ENABLE_TESTS})

add_subdirectory(src)

if(PSF2C_ENABLE_TESTS)
    add_subdirectory(tests)
endif()

