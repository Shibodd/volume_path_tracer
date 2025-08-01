add_library (rpng STATIC src/rpng.cpp)
target_compile_definitions (rpng PRIVATE RPNG_IMPLEMENTATION RPNG_DEFLATE_IMPLEMENTATION)
target_include_directories (rpng PUBLIC include)
set_target_properties (rpng PROPERTIES LINKER_LANGUAGE CXX)