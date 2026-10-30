add_library(hypermaze_model STATIC
    src/world/Graph.cpp src/world/Terrain.cpp src/world/HyperMaze.cpp src/world/Visibility.cpp)
target_include_directories(hypermaze_model PUBLIC include)

add_executable(hypermaze
    src/main.cpp src/Hypermaze.cpp src/controllers/player.cpp src/gui/GameUI.cpp)
target_link_libraries(hypermaze PRIVATE hypermaze_model college_ports_common)

set_target_properties(hypermaze PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    BUILD_RPATH_USE_ORIGIN TRUE)
college_ports_copy_runtime_dlls(hypermaze)

if(BUILD_TESTING)
    add_executable(hypermaze_tests tests/world.cpp)
    target_link_libraries(hypermaze_tests PRIVATE hypermaze_model)
    add_test(NAME hypermaze_world COMMAND hypermaze_tests)
    add_executable(hypermaze_visibility_tests tests/visibility.cpp)
    target_link_libraries(hypermaze_visibility_tests PRIVATE hypermaze_model)
    add_test(NAME hypermaze_visibility COMMAND hypermaze_visibility_tests)
endif()
