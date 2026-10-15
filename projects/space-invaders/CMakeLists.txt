project(space_invaders LANGUAGES CXX)

file(GLOB_RECURSE space_invaders_sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE space_invaders_headers CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")

add_executable(space_invaders
    ${space_invaders_sources}
    ${space_invaders_headers})

target_include_directories(space_invaders PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(space_invaders PRIVATE Cheryl::Cheryl)

set_target_properties(space_invaders PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
