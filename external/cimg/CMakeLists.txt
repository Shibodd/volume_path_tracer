find_package(X11 REQUIRED)

set (CIMG_SYSTEM_INCLUDES ${X11_INCLUDE_DIR})
set (CIMG_SYSTEM_LIBS ${X11_LIBRARIES})

add_library (cimg INTERFACE)
target_include_directories (cimg INTERFACE include ${CIMG_SYSTEM_INCLUDES})
target_link_libraries(cimg INTERFACE ${CIMG_SYSTEM_LIBS})