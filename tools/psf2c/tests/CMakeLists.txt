add_executable(psf2c_tests
    psf_test.cpp
    file_io_test.cpp
    codegen_test.cpp
)

target_compile_features(psf2c_tests PRIVATE cxx_std_17)

target_link_libraries(psf2c_tests PRIVATE
    psf2c_lib
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
)

target_compile_options(psf2c_tests PRIVATE
    -Wall
    -Wextra
    -O2
)

# Copy test fixtures to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/fixtures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

include(GoogleTest)
gtest_discover_tests(psf2c_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
