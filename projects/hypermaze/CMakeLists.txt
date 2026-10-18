file(GLOB_RECURSE hypermaze_sources CONFIGURE_DEPENDS
    "src/*.cpp"
    "include/*.h"
    "include/*.hpp")

add_executable(hypermaze ${hypermaze_sources})
target_include_directories(hypermaze PRIVATE include)

target_link_libraries(hypermaze PRIVATE
    Cheryl::OpenGL::Startup)

set_target_properties(hypermaze PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    BUILD_RPATH_USE_ORIGIN TRUE)
college_ports_copy_runtime_dlls(hypermaze)
