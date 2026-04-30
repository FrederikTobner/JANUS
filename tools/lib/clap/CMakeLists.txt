cmake_minimum_required(VERSION 3.20)
project(clap VERSION 0.1.0 LANGUAGES C)

add_library(clap STATIC
    src/clap.c
    src/help.c
)

target_include_directories(clap
    PUBLIC  include
    PRIVATE src
)

target_compile_features(clap PUBLIC c_std_11)

target_compile_options(clap PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
