add_library(word_model STATIC src/gameplay/WordBox.cpp src/gameplay/WordSearch.cpp)
target_include_directories(word_model PUBLIC include)

add_executable(word_search
    src/main.cpp src/WordGame.cpp src/controllers/controls.cpp src/gui/GameUI.cpp)
target_link_libraries(word_search PRIVATE word_model college_ports_common)
target_compile_definitions(word_search PRIVATE
    WORD_DEFAULT_PUZZLE="${CMAKE_CURRENT_SOURCE_DIR}/data/default.word")

set_target_properties(word_search PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    BUILD_RPATH_USE_ORIGIN TRUE)
college_ports_copy_runtime_dlls(word_search)

if(BUILD_TESTING)
    add_executable(word_tests tests/search.cpp)
    target_link_libraries(word_tests PRIVATE word_model)
    target_compile_definitions(word_tests PRIVATE WORD_DEFAULT_PUZZLE="${CMAKE_CURRENT_SOURCE_DIR}/data/default.word")
    add_test(NAME word_search_model COMMAND word_tests)
endif()
