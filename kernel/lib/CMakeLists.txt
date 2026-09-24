list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Library)

# Build all library modules 
add_subdirectory(gfx)
add_subdirectory(fmt)
add_subdirectory(page_tables)
add_subdirectory(mem)
