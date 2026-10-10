cmake_minimum_required(VERSION 3.28)

project(college_ports LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CHERYL_ENGINE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../cheryl-engine"
    CACHE PATH "Path to a local Cheryl Engine v4 checkout (empty to fetch it)")
set(CHERYL_ENGINE_GIT_REPOSITORY "https://github.com/cppcooper/cheryl-engine.git"
    CACHE STRING "Repository used when Cheryl Engine must be fetched")
set(CHERYL_ENGINE_GIT_TAG "engine-foundations-and-consumer-facilities"
    CACHE STRING "Git ref used when Cheryl Engine must be fetched")

set(COLLEGE_PORTS_PROJECTS "space-invaders" CACHE STRING
    "Projects to configure: all, hypermaze, rover, space-invaders, word (or a semicolon-separated list)")
set_property(CACHE COLLEGE_PORTS_PROJECTS PROPERTY STRINGS
    all hypermaze rover space-invaders word)
set(_college_ports_available_projects hypermaze rover space-invaders word)
if(COLLEGE_PORTS_PROJECTS STREQUAL "all")
    set(_college_ports_selected_projects ${_college_ports_available_projects})
else()
    set(_college_ports_selected_projects ${COLLEGE_PORTS_PROJECTS})
    if(NOT _college_ports_selected_projects)
        message(FATAL_ERROR "COLLEGE_PORTS_PROJECTS must select at least one project or be set to 'all'.")
    endif()
    foreach(_project IN LISTS _college_ports_selected_projects)
        if(NOT _project IN_LIST _college_ports_available_projects)
            message(FATAL_ERROR
                "Unknown project '${_project}'. Available projects: ${_college_ports_available_projects}")
        endif()
    endforeach()
    list(REMOVE_DUPLICATES _college_ports_selected_projects)
endif()

# Build only the engine components needed by these project targets.
option(CHERYL_BUILD_NATIVE_GLFW "Build Cheryl's native GLFW integration" ON)
option(CHERYL_BUILD_OPENGL "Build Cheryl's OpenGL backend" ON)
option(CHERYL_NATIVE_INPUT "Include Cheryl's native input adapter" ON)
option(CHERYL_BUILD_UI_TGUI "Build Cheryl's optional TGUI integration" OFF)
option(CHERYL_BUILD_UI_RMLUI "Build Cheryl's optional RmlUi integration" OFF)
option(CHERYL_BUILD_AUDIO_MINIAUDIO "Build Cheryl's optional miniaudio integration" OFF)
option(CHERYL_BUILD_DEMO "Build Cheryl's demonstration app" OFF)
option(CHERYL_BUILD_TESTS "Build Cheryl's tests and acceptance tools" OFF)
option(CHERYL_BUILD_ALL_TESTS "Build Cheryl's aggregate test runners" OFF)
option(CHERYL_BUILD_ACCEPTANCE_TESTS "Build Cheryl's acceptance runners" OFF)
option(CHERYL_BUILD_CONSUMER_TESTS "Build Cheryl's consumer checks" OFF)

if(CHERYL_ENGINE_SOURCE_DIR AND EXISTS "${CHERYL_ENGINE_SOURCE_DIR}/CMakeLists.txt")
    message(STATUS "Using Cheryl Engine checkout: ${CHERYL_ENGINE_SOURCE_DIR}")
    add_subdirectory("${CHERYL_ENGINE_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/cheryl-engine")
else()
    include(FetchContent)
    message(STATUS "Fetching Cheryl Engine ${CHERYL_ENGINE_GIT_TAG}")
    FetchContent_Declare(cheryl_engine
        GIT_REPOSITORY "${CHERYL_ENGINE_GIT_REPOSITORY}"
        GIT_TAG "${CHERYL_ENGINE_GIT_TAG}"
        GIT_SHALLOW TRUE
        GIT_SUBMODULES_RECURSE TRUE)
    FetchContent_MakeAvailable(cheryl_engine)
endif()

if(NOT TARGET Cheryl::OpenGL::Startup)
    message(FATAL_ERROR
        "The selected college projects require Cheryl's OpenGL startup target. "
        "Enable CHERYL_BUILD_NATIVE_GLFW and CHERYL_BUILD_OPENGL.")
endif()

foreach(_project IN LISTS _college_ports_selected_projects)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/projects/${_project}")
endforeach()
