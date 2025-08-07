add_subdirectory (spng)
add_subdirectory (pcg-cpp)

set (NANOVDB_BUILD_TOOLS OFF CACHE INTERNAL "Disable tools")
set (NANOVDB_USE_ZLIB ON CACHE INTERNAL "Enable support for zip")
add_subdirectory (openvdb/nanovdb/nanovdb)