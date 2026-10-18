# Stage the DLLs known to CMake beside a Windows executable. Linux and macOS use
# the runtime search paths generated from each target's linked libraries.
function(college_ports_copy_runtime_dlls target)
    if(WIN32)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy -t "$<TARGET_FILE_DIR:${target}>"
                "$<TARGET_RUNTIME_DLLS:${target}>"
            COMMAND_EXPAND_LISTS
            VERBATIM)
    endif()
endfunction()
