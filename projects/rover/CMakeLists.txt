file(GLOB_RECURSE rover_sources CONFIGURE_DEPENDS
    "src/*.cpp"
    "include/*.h"
    "include/*.hpp")

add_executable(rover ${rover_sources})
target_include_directories(rover PRIVATE include)

target_link_libraries(rover PRIVATE
    Cheryl::OpenGL::Startup)

set_target_properties(rover PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    BUILD_RPATH_USE_ORIGIN TRUE)
college_ports_copy_runtime_dlls(rover)
