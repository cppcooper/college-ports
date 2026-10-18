get_filename_component(college_ports_source_dir "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
set(CHERYL_ENGINE_SOURCE_DIR "${college_ports_source_dir}/extern/cheryl-engine"
    CACHE PATH "Buildable Cheryl Engine source used by the cheryl-sdk target")
get_filename_component(CHERYL_ENGINE_SOURCE_DIR "${CHERYL_ENGINE_SOURCE_DIR}"
    ABSOLUTE BASE_DIR "${college_ports_source_dir}")
string(SHA256 cheryl_source_id "${CHERYL_ENGINE_SOURCE_DIR}")
string(SUBSTRING "${cheryl_source_id}" 0 8 cheryl_source_id)
set(cheryl_build_dir "${college_ports_source_dir}/builds/cheryl/${CMAKE_BUILD_TYPE}/build-${cheryl_source_id}")
set(cheryl_sdk_prefix "${college_ports_source_dir}/builds/cheryl/${CMAKE_BUILD_TYPE}/sdk")
set(cheryl_source_args
    "-DCOLLEGE_PORTS_SOURCE_DIR:PATH=${college_ports_source_dir}"
    "-DCHERYL_ENGINE_SOURCE_DIR:PATH=${CHERYL_ENGINE_SOURCE_DIR}")

add_custom_target(cheryl-source-checkout
    COMMAND "${CMAKE_COMMAND}" ${cheryl_source_args} -DCHECKOUT_CHERYL_SOURCE=TRUE
        -P "${CMAKE_CURRENT_LIST_DIR}/InitializeCheryl.cmake"
    COMMENT "Check out the Cheryl commit recorded by the submodule"
    USES_TERMINAL
    VERBATIM)

# Build a separate SDK using Cheryl's defaults and the active compiler/toolchain.
set(cheryl_cmake_args
    "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
    "-DCMAKE_INSTALL_PREFIX:PATH=${cheryl_sdk_prefix}")
if(CMAKE_VERSION VERSION_GREATER_EQUAL 4)
    # Some of Cheryl's dependencies still need this compatibility setting.
    list(APPEND cheryl_cmake_args "-DCMAKE_POLICY_VERSION_MINIMUM:STRING=3.5")
endif()
foreach(setting IN ITEMS CMAKE_C_COMPILER CMAKE_CXX_COMPILER CMAKE_TOOLCHAIN_FILE)
    if(${setting})
        list(APPEND cheryl_cmake_args "-D${setting}:FILEPATH=${${setting}}")
    endif()
endforeach()
if(CMAKE_GENERATOR STREQUAL "Ninja")
    list(APPEND cheryl_cmake_args "-DCMAKE_MAKE_PROGRAM:FILEPATH=${CMAKE_MAKE_PROGRAM}")
endif()

add_custom_target(cheryl-sdk
    COMMAND "${CMAKE_COMMAND}" ${cheryl_source_args}
        -P "${CMAKE_CURRENT_LIST_DIR}/InitializeCheryl.cmake"
    COMMAND "${CMAKE_COMMAND}" -S "${CHERYL_ENGINE_SOURCE_DIR}"
        -B "${cheryl_build_dir}" -G Ninja ${cheryl_cmake_args}
    COMMAND "${CMAKE_COMMAND}" --build "${cheryl_build_dir}" --target cheryl-sdk
        --config "${CMAKE_BUILD_TYPE}"
    COMMAND "${CMAKE_COMMAND}" --install "${cheryl_build_dir}" --component CherylSDK
        --prefix "${cheryl_sdk_prefix}" --config "${CMAKE_BUILD_TYPE}"
    COMMAND "${CMAKE_COMMAND}" -E echo "Local SDK installed. Reload CMake Project to refresh SDK discovery."
    COMMENT "Build and install the local Cheryl SDK"
    USES_TERMINAL
    VERBATIM)

# Normal package discovery (including system SDKs) precedes this local fallback.
find_package(Cheryl CONFIG QUIET PATHS "${cheryl_sdk_prefix}")
set(CHERYL_SDK_AVAILABLE "${Cheryl_FOUND}")
if(Cheryl_FOUND)
    message(STATUS "Using Cheryl SDK: ${Cheryl_DIR}")
else()
    message(STATUS
        "No installed Cheryl SDK found.\n"
        "Build the cheryl-sdk target, then Reload CMake Project to enable game targets.")
endif()
