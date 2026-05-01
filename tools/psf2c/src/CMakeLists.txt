find_package(ZLIB REQUIRED)

# Internal static library reused by tests — not registered as a public tool library.
add_library(psf2c_lib STATIC
    codegen.c
    file_io.c
    psf.c
)
target_include_directories(psf2c_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(psf2c_lib PRIVATE -Wall -Wextra -O2 -std=c11)
target_link_libraries(psf2c_lib PUBLIC ZLIB::ZLIB)

tools_add_tool(psf2c
    SOURCES main.c
    DEPENDENCIES psf2c_lib clap
)