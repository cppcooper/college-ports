cmake_minimum_required(VERSION 3.28)

project(college_ports LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

# Projects
# Select one game, a semicolon-separated list, or all four.
set(COLLEGE_PORTS_PROJECTS "space-invaders" CACHE STRING "Games to configure (or 'all')")
set(college_ports_projects hypermaze rover space-invaders word)
set_property(CACHE COLLEGE_PORTS_PROJECTS PROPERTY STRINGS all ${college_ports_projects})

if(COLLEGE_PORTS_PROJECTS STREQUAL "all")
    set(selected_projects ${college_ports_projects})
else()
    set(selected_projects ${COLLEGE_PORTS_PROJECTS})
endif()
if(NOT selected_projects)
    message(FATAL_ERROR "COLLEGE_PORTS_PROJECTS must select at least one game or 'all'.")
endif()
foreach(game IN LISTS selected_projects)
    if(NOT game IN_LIST college_ports_projects)
        message(FATAL_ERROR "Unknown game '${game}'. Choose from: ${college_ports_projects}")
    endif()
endforeach()
list(REMOVE_DUPLICATES selected_projects)

# Installed dependencies and runtime setup
include(cmake/Cheryl.cmake)
if(NOT CHERYL_SDK_AVAILABLE)
    return()
endif()
include(cmake/GameRuntime.cmake)

# Game targets
add_subdirectory(projects/common)
foreach(game IN LISTS selected_projects)
    add_subdirectory("projects/${game}")
endforeach()
