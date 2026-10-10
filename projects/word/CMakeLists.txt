project(word_search LANGUAGES CXX)

file(GLOB_RECURSE word_search_sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE word_search_headers CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")

add_executable(word_search
    ${word_search_sources}
    ${word_search_headers})

target_include_directories(word_search PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(word_search PRIVATE Cheryl::OpenGL::Startup)

set_target_properties(word_search PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
