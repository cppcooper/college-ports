file(GLOB_RECURSE word_search_sources CONFIGURE_DEPENDS
    "src/*.cpp"
    "include/*.h"
    "include/*.hpp")

add_executable(word_search ${word_search_sources})
target_include_directories(word_search PRIVATE include)

target_link_libraries(word_search PRIVATE
    Cheryl::OpenGL::Startup)

set_target_properties(word_search PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    BUILD_RPATH_USE_ORIGIN TRUE)
college_ports_copy_runtime_dlls(word_search)
