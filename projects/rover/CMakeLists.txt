project(rover LANGUAGES CXX)

file(GLOB_RECURSE rover_sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE rover_headers CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")

add_executable(rover
    ${rover_sources}
    ${rover_headers})

target_include_directories(rover PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(rover PRIVATE Cheryl::OpenGL::Startup)

set_target_properties(rover PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
