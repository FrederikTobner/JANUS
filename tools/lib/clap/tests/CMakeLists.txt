add_executable(clap_tests
    clap_test.cpp
)

target_compile_features(clap_tests PRIVATE cxx_std_17)

target_link_libraries(clap_tests PRIVATE
    clap
    GTest::gtest
    GTest::gtest_main
)

target_compile_options(clap_tests PRIVATE
    -Wall
    -Wextra
    -O2
)

include(GoogleTest)
gtest_discover_tests(clap_tests)
