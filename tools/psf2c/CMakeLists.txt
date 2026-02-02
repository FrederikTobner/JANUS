# Tools are built with the host compiler, not the cross-compiler
# This allows them to run on the build machine

find_package(ZLIB REQUIRED)

add_executable(psf2c
    main.c
    psf.c
    file_io.c
    codegen.c
)

# Use the host compiler settings
target_compile_options(psf2c PRIVATE
    -Wall
    -Wextra
    -O2
    -std=c11
)

target_link_libraries(psf2c PRIVATE ZLIB::ZLIB)

# Install target (optional)
install(TARGETS psf2c
    RUNTIME DESTINATION bin
)

