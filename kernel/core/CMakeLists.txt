list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Core)

add_subdirectory(kio)
add_subdirectory(kmalloc)
