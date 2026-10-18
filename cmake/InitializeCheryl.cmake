find_package(Git REQUIRED)
execute_process(
    COMMAND "${GIT_EXECUTABLE}" -C "${COLLEGE_PORTS_SOURCE_DIR}"
        rev-parse :extern/cheryl-engine
    OUTPUT_VARIABLE required_revision OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY)

# Initialize the default submodule on demand; custom source paths must already exist.
if(CHERYL_ENGINE_SOURCE_DIR STREQUAL "${COLLEGE_PORTS_SOURCE_DIR}/extern/cheryl-engine")
    if(NOT EXISTS "${CHERYL_ENGINE_SOURCE_DIR}/CMakeLists.txt")
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" -C "${COLLEGE_PORTS_SOURCE_DIR}"
                submodule update --init --depth 1 -- extern/cheryl-engine
            COMMAND_ERROR_IS_FATAL ANY)
    endif()
endif()

if(NOT EXISTS "${CHERYL_ENGINE_SOURCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "No buildable Cheryl source at '${CHERYL_ENGINE_SOURCE_DIR}'. "
        "Set CHERYL_ENGINE_SOURCE_DIR to an existing Cheryl checkout.")
endif()

execute_process(
    COMMAND "${GIT_EXECUTABLE}" -C "${CHERYL_ENGINE_SOURCE_DIR}" rev-parse HEAD
    OUTPUT_VARIABLE source_revision OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY)
if(NOT source_revision STREQUAL required_revision)
    if(NOT CHECKOUT_CHERYL_SOURCE)
        message(FATAL_ERROR
            "Cheryl source is at ${source_revision}; the submodule records ${required_revision}.\n"
            "Build cheryl-source-checkout to align CHERYL_ENGINE_SOURCE_DIR, then build cheryl-sdk.\n"
            "To change the project's required commit instead, stage extern/cheryl-engine after selecting it.")
    endif()
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" -C "${CHERYL_ENGINE_SOURCE_DIR}"
            status --porcelain --untracked-files=no
        OUTPUT_VARIABLE source_changes OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND_ERROR_IS_FATAL ANY)
    if(source_changes)
        message(FATAL_ERROR "Save changes in '${CHERYL_ENGINE_SOURCE_DIR}' before changing its commit.")
    endif()
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" -C "${CHERYL_ENGINE_SOURCE_DIR}"
            cat-file -e "${required_revision}^{commit}"
        RESULT_VARIABLE missing_revision OUTPUT_QUIET ERROR_QUIET)
    if(missing_revision)
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" -C "${CHERYL_ENGINE_SOURCE_DIR}" fetch origin "${required_revision}"
            COMMAND_ERROR_IS_FATAL ANY)
    endif()
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" -C "${CHERYL_ENGINE_SOURCE_DIR}" checkout --detach "${required_revision}"
        COMMAND_ERROR_IS_FATAL ANY)
endif()

if(CHECKOUT_CHERYL_SOURCE OR
        CHERYL_ENGINE_SOURCE_DIR STREQUAL "${COLLEGE_PORTS_SOURCE_DIR}/extern/cheryl-engine")
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" -C "${CHERYL_ENGINE_SOURCE_DIR}"
            submodule update --init --recursive --depth 1
        COMMAND_ERROR_IS_FATAL ANY)
endif()
message(STATUS "Cheryl source matches the submodule commit: ${required_revision}")
