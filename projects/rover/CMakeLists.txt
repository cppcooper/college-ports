add_library(rover_model STATIC
    src/levels/tile-map.cpp src/levels/viewport.cpp src/levels/map-editor.cpp src/actors/rover.cpp)
target_include_directories(rover_model PUBLIC include)

add_executable(rover
    src/main.cpp src/RoverGame.cpp src/controllers/controls.cpp src/gui/GameUI.cpp)
target_link_libraries(rover PRIVATE rover_model college_ports_common)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/data")
target_compile_definitions(rover PRIVATE
    ROVER_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
    ROVER_SAVE_FILE="${CMAKE_CURRENT_BINARY_DIR}/data/edited.map")

set_target_properties(rover PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    BUILD_RPATH_USE_ORIGIN TRUE)
college_ports_copy_runtime_dlls(rover)

if(BUILD_TESTING)
    add_executable(rover_tests tests/pathfinding.cpp)
    target_link_libraries(rover_tests PRIVATE rover_model)
    target_compile_definitions(rover_tests PRIVATE ROVER_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
    add_test(NAME rover_pathfinding COMMAND rover_tests)
endif()
