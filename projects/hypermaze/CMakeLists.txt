project(hypermaze LANGUAGES CXX)

file(GLOB_RECURSE hypermaze_sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE hypermaze_headers CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")

add_executable(hypermaze
    ${hypermaze_sources}
    ${hypermaze_headers})

target_include_directories(hypermaze PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(hypermaze PRIVATE Cheryl::OpenGL::Startup)

set_target_properties(hypermaze PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
