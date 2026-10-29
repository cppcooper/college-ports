file(GLOB_RECURSE space_invaders_sources CONFIGURE_DEPENDS
    "src/*.cpp"
    "include/*.h"
    "include/*.hpp")

include_directories("include")
add_executable(space_invaders ${space_invaders_sources})
target_include_directories(space_invaders PRIVATE include)

target_link_libraries(space_invaders PRIVATE
    Cheryl::OpenGL::Startup
    Cheryl::UI::TGUI
    Cheryl::UI::RmlUi)

# Seed once; reconfiguration must preserve scores saved during play.
set(space_invaders_score_file "${CMAKE_CURRENT_BINARY_DIR}/data/highscore.txt")
if(NOT EXISTS "${space_invaders_score_file}")
    configure_file(assets/highscore.txt "${space_invaders_score_file}" COPYONLY)
endif()
target_compile_definitions(space_invaders PRIVATE
    SPACE_INVADERS_SCORE_FILE="${space_invaders_score_file}")

set_target_properties(space_invaders PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    BUILD_RPATH_USE_ORIGIN TRUE)
college_ports_copy_runtime_dlls(space_invaders)
