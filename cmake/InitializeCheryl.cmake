find_package(Git REQUIRED)
get_filename_component(cheryl_default_source
    "${COLLEGE_PORTS_SOURCE_DIR}/extern/cheryl-engine" REALPATH)
get_filename_component(cheryl_selected_source "${CHERYL_ENGINE_SOURCE_DIR}" REALPATH
    BASE_DIR "${COLLEGE_PORTS_SOURCE_DIR}")
execute_process(
    COMMAND "${GIT_EXECUTABLE}" -C "${COLLEGE_PORTS_SOURCE_DIR}"
        rev-parse :./extern/cheryl-engine
    OUTPUT_VARIABLE required_revision OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE cheryl_revision_result ERROR_QUIET)
if(NOT cheryl_revision_result EQUAL 0)
    if(CHECKOUT_CHERYL_SOURCE)
        message(FATAL_ERROR
            "No recorded Cheryl submodule commit is available to check out. "
            "Select an existing external checkout with CHERYL_ENGINE_SOURCE_DIR instead.")
    endif()
    if(cheryl_selected_source STREQUAL cheryl_default_source)
        message(FATAL_ERROR
            "The default Cheryl source requires the college-ports Git submodule entry. "
            "For source-only deployments, set CHERYL_ENGINE_SOURCE_DIR to an existing external Git checkout.")
    endif()
endif()

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
    COMMAND "${GIT_EXECUTABLE}" -C "${CHERYL_ENGINE_SOURCE_DIR}"
        rev-parse --verify "HEAD^{commit}"
    OUTPUT_VARIABLE source_revision OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE cheryl_source_result ERROR_QUIET)
if(NOT cheryl_source_result EQUAL 0)
    message(FATAL_ERROR
        "Cheryl source at '${CHERYL_ENGINE_SOURCE_DIR}' is not a Git checkout with a valid HEAD.")
endif()
if(NOT cheryl_revision_result EQUAL 0)
    set(required_revision "${source_revision}")
endif()
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
if(cheryl_revision_result EQUAL 0)
    message(STATUS "Cheryl source matches the submodule commit: ${required_revision}")
else()
    message(STATUS
        "No consumer Git submodule entry is available; building explicitly selected external Cheryl checkout at ${source_revision}")
endif()
