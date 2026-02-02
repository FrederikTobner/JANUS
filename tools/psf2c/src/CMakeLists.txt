find_package(ZLIB REQUIRED)

# Core library (for reuse in tests)
file(GLOB PSF2C_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
list(FILTER PSF2C_LIB_SOURCES EXCLUDE REGEX "main\\.c$")

add_library(psf2c_lib STATIC 
    codegen.c
    file_io.c
    psf.c
)
target_include_directories(psf2c_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(psf2c_lib PRIVATE
    -Wall
    -Wextra
    -O2
    -std=c11
)
target_link_libraries(psf2c_lib PUBLIC ZLIB::ZLIB)

# Main executable
add_executable(psf2c main.c)
target_compile_options(psf2c PRIVATE
    -Wall
    -Wextra
    -O2
    -std=c11
)
target_link_libraries(psf2c PRIVATE psf2c_lib)

# Install target (optional)
install(TARGETS psf2c
    RUNTIME DESTINATION bin
)